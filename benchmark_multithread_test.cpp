#include <algorithm>
#include <chrono>
#include <sstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include <array>

#include "exchange.h"
#include "orderbook.h"
#include "test.h"

struct TestListener : OrderBookListener {
    int tradeCount=0;
    void onTrade(const Trade& trade) override {
        tradeCount++;
    }
};

void insertOrdersWithTrades() {
    static std::array<std::string,4> instruments{ "i1","i2","i3","i4" };

    static const int N_THREADS=4;
    static const int N_ORDERS = 5000000;
    static const int TOTAL_ORDERS = N_ORDERS * 2 * N_THREADS;

    struct MyExchangeListener : public ExchangeListener {
        std::atomic<long> tradeCount = 0;
        void onTrade(const Trade& trade) override {
            tradeCount++;
        }
    } listener;

    Exchange exchange(listener);
    auto fn = [&exchange](const std::string &instrument) {
        for(int i=0;i<N_ORDERS;i++) {
            exchange.buy(instrument,5000.0 + 1 * (i%1000),10,"");
        }
        for(int i=0;i<N_ORDERS;i++) {
            exchange.sell(instrument,5000.0 + 1 * (i%1000),10,"");
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
    std::cout << "multithread, insert orders with trades, " << listener.tradeCount << " trades\n";
}


int main(int argc,char **argv) {
    std::cout << "sizeof Fixed " << sizeof(F) << "\n";
    insertOrdersWithTrades();
}