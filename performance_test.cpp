#include <algorithm>
#include <chrono>
#include <sstream>
#include <iostream>
#include <random>
#include <stdexcept>

#include "exchange.h"
#include "orderbook.h"
#include "test.h"

struct TestListener : OrderBookListener {
    int tradeCount=0;
    void onTrade(const Trade& trade) override {
        tradeCount++;
    }
};

void insertOrders() {
    TestListener listener;
    OrderBook ob(listener);

    static const int N_ORDERS = 5000000;

    auto start = std::chrono::system_clock::now();

    for(int i=0;i<N_ORDERS;i++) {
        ob.insertOrder(new TestOrder(i,5000.0 - 1 * (i%1000),10,BUY));
    }
    for(int i=0;i<N_ORDERS;i++) {
        ob.insertOrder(new TestOrder(N_ORDERS+i,50001.0 + 1 * (i%1000),10,SELL));
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << "process orders " << duration.count() << " usecs, usec per order " << (duration.count()/(double)(N_ORDERS*2)) << ", orders per sec " << (int)(((N_ORDERS*2)/(duration.count()/1000000.0))) << "\n";
    if(listener.tradeCount!=0) throw std::runtime_error("trade count should be 0");
}

void insertOrdersWithTrades() {

    TestListener listener;
    OrderBook ob(listener);

    static const int N_ORDERS = 5000000;

    auto start = std::chrono::system_clock::now();

    for(int i=0;i<N_ORDERS;i++) {
        ob.insertOrder(new TestOrder(i,5000.0 + 1 * (i%1000),10,BUY));
    }
    for(int i=0;i<N_ORDERS;i++) {
        ob.insertOrder(new TestOrder(N_ORDERS+i,5000.0 + 1 * (i%1000),10,SELL));
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << "insert orders with trades " << duration.count() << " usecs, usec per order " << (duration.count()/(double)(N_ORDERS*2)) << ", orders per sec " << (int)(((N_ORDERS*2)/(duration.count()/1000000.0))) << "\n";
    std::cout << "insert orders with trades, " << listener.tradeCount << " trades\n";
}

/** tests the time to remove an order at a random position in the OrderBook */
void cancelOrders() {
    OrderBookListener listener;
    OrderBook ob(listener);

    static const int N_ORDERS = 1000000;

    std::vector<std::string> output;

    Order* orders[N_ORDERS];

    for(int i=0;i<N_ORDERS;i++) {
        auto order = new TestOrder(i,100.0 + 1 * (i%1000),10,BUY);
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

    std::cout << "cancel orders " << duration.count() << " usecs, usec per order " << (duration.count()/(double)(N_ORDERS)) << "\n";
}

int main(int argc,char **argv) {
    insertOrders();
    insertOrdersWithTrades();
    cancelOrders();
}