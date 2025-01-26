#include <algorithm>
#include <chrono>
#include <sstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <array>
#include <vector>

#include "exchange.h"
#include "order.h"
#include "orderbook.h"
#include "test.h"

void insertOrders(const bool withTrades) {
    static const int N_THREADS=std::thread::hardware_concurrency();
    static std::array<std::string,16> instruments;

    for(int i=0;i<N_THREADS;i++) instruments[i] = "i"+std::to_string(i+1);

    static const int N_ORDERS = 250000;
    static const int TOTAL_ORDERS = N_ORDERS * 2 * N_THREADS;

    struct MyExchangeListener : public ExchangeListener {
        std::atomic<long> tradeCount = 0;
        void onTrade(const Trade& trade) override {
            tradeCount++;
        }
    } listener;

    Exchange exchange(listener);
    const std::string session("dummy");
    auto fn = [&exchange,session,withTrades](const std::string &instrument) {
        for(int i=0;i<N_ORDERS;i++) {
            exchange.buy(session,instrument,5000.0 + 1 * (i%1000),10,"");
        }
        for(int i=0;i<N_ORDERS;i++) {
            exchange.sell(session,instrument,(withTrades ? 5000.0 : 10000.0) + 1 * (i%1000),10,"");
        }
    };

    auto start = std::chrono::system_clock::now();

    std::vector<std::thread> threads;
    for(int i=0;i<N_THREADS;i++) {
        threads.push_back(std::thread(fn,instruments[i]));
    }
    for(auto itr = threads.begin(); itr != threads.end(); itr++) {
        itr->join();
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << "multithread, insert orders with trades, usec per order " << (duration.count()/(double)(TOTAL_ORDERS)) << ", orders per sec " << (int)(((TOTAL_ORDERS)/(duration.count()/1000000.0))) << "\n";
    std::cout << "multithread, insert orders with trade match % " << (listener.tradeCount*100/TOTAL_ORDERS) << "\n";
}

/** tests the time to remove an order at a random position in the OrderBook */
void cancelOrders() {
    static const int N_THREADS=std::thread::hardware_concurrency();
    static std::array<std::string,16> instruments;

    for(int i=0;i<N_THREADS;i++) instruments[i] = "i"+std::to_string(i+1);

    static const int N_ORDERS = 250000;
    static const int TOTAL_ORDERS = N_ORDERS * N_THREADS;

    std::vector<std::string> output;

    std::vector<std::vector<long>> oids(N_THREADS,std::vector<long>(N_ORDERS));

    struct MyExchangeListener : public ExchangeListener {
        std::atomic<long> tradeCount = 0;
        void onTrade(const Trade& trade) override {
            tradeCount++;
        }
    } listener;

    Exchange exchange(listener);
    const std::string session("dummy");

    for(int t=0;t<N_THREADS;t++) {
        for(int i=0;i<N_ORDERS;i++) {
            auto oid = exchange.buy(session,instruments[t], 100.0 + 1 * (i%1000), 10, dummy_oid);
            oids[t][i]=oid;
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());

    for(int t=0;t<N_THREADS;t++) {
        std::shuffle(std::begin(oids[t]),std::end(oids[t]),g);
    }

    auto start = std::chrono::system_clock::now();
    std::vector<std::thread> threads;

    auto fn = [&](const int tid) {
        for(int i=0;i<N_ORDERS;i++) {
            exchange.cancel(oids[tid][i],session);
        }
    };

    for(int i=0;i<N_THREADS;i++) {
        threads.push_back(std::thread(fn,i));
    }

    for(auto thread = threads.begin();thread!=threads.end();thread++) {
        thread->join();
    }

    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

    std::cout << "cancel orders, usec per order " << (duration.count()/(double)(TOTAL_ORDERS)) << ", orders per sec " << (int)(((TOTAL_ORDERS)/(duration.count()/1000000.0))) << "\n";
}

int main(int argc,char **argv) {
    std::cout << "sizeof Fixed " << sizeof(F) << " number of cores " << std::thread::hardware_concurrency() << "\n";
    insertOrders(false);
    insertOrders(true);
    cancelOrders();
}