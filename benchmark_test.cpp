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

void insertOrders(const bool withTrades,const int PRICE_LEVELS) {

    TestListener listener;
    OrderBook ob(dummy_instrument,listener);

    static const int N_ORDERS = 5000000;
    static const int TOTAL_ORDERS = N_ORDERS * 2;

    auto start = std::chrono::system_clock::now();

    for(int i=0;i<N_ORDERS;i++) {
        ob.insertOrder(new (ob.allocateOrder()) TestOrder(i,5000.0 + 1 * (i%PRICE_LEVELS),10,BUY));
    }
    for(int i=0;i<N_ORDERS;i++) {
        ob.insertOrder(new (ob.allocateOrder()) TestOrder(N_ORDERS+i,(withTrades ? 5000.0 : 10000.0) + 1 * (i%PRICE_LEVELS),10,SELL));
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << "insert orders " << PRICE_LEVELS << " levels, usec per order " << (duration.count()/(double)(N_ORDERS*2)) << ", orders per sec " << (int)(((N_ORDERS*2)/(duration.count()/1000000.0))) << "\n";
    std::cout << "insert orders " << PRICE_LEVELS << " levels with trade match % " << (listener.tradeCount*100/TOTAL_ORDERS) << "\n";
}

/** tests the time to remove an order at a random position in the OrderBook */
void cancelOrders(const int PRICE_LEVELS) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument,listener);

    static const int N_ORDERS = 1000000;

    std::vector<std::string> output;

    Order* orders[N_ORDERS];

    for(int i=0;i<N_ORDERS;i++) {
        auto order = new (ob.allocateOrder()) TestOrder(i,100.0 + 1 * (i%PRICE_LEVELS),10,BUY);
        ob.insertOrder(order);
        orders[i]=order;
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(std::begin(orders),std::end(orders),g);

    auto start = std::chrono::system_clock::now();
    for(int i=0;i<N_ORDERS;i++) {
        ob.cancelOrder(orders[i]);
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

    std::cout << "cancel orders "<<PRICE_LEVELS<<" levels, usec per order " << (duration.count()/(double)(N_ORDERS)) << ", orders per sec " << (int)(((N_ORDERS)/(duration.count()/1000000.0))) << "\n";
}

int main(int argc,char **argv) {
    std::cout << "sizeof Fixed " << sizeof(F) << " number of cores " << std::thread::hardware_concurrency() << "\n";
    insertOrders(false,1000);
    insertOrders(true,1000);
    cancelOrders(1000);
    insertOrders(false,10);
    insertOrders(true,10);
    cancelOrders(10);
}