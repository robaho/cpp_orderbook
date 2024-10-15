#pragma once

#include <string>
#include <unordered_map>

#include "order.h"
#include "orderbook.h"
#include "books.h"
#include "spinlock.h"

struct ExchangeListener {
    /** callback when order properties change */
    virtual void onOrder(const Order& order){};
    /** callback when trade occurs */
    virtual void onTrade(const Trade& trade){};
};

class Exchange : OrderBookListener {
public:
    Exchange();
    Exchange(ExchangeListener& listener);
    /** submit limit buy order. returns exchange order id */
    long buy(std::string instrument,F price,int quantity,std::string orderId);
    /** submit market buy order. returns exchange order id. If the order cannot be filled, the rest is cancelled */
    long marketBuy(std::string instrument,int quantity,std::string orderId) {
        return buy(instrument,DBL_MAX,quantity,orderId);
    }
    /** submit limit sell order. returns exchange order id */
    long sell(std::string instrument,F price,int quantity,std::string orderId);
    /** submit market sell order. returns exchange order id. If the order cannot be filled, the rest is cancelled */
    long marketSell(std::string instrument,int quantity,std::string orderId) {
        return sell(instrument,-DBL_MAX,quantity,orderId);
    }
    int cancel(long exchangeId);
    const Book book(const std::string& instrument);
    const Order getOrder(long exchangeId);
    void onOrder(const Order& order) override {
        listener.onOrder(order);
    }
    void onTrade(const Trade& trade) override {
        listener.onTrade(trade);
    }
    Guard lock() {
        return mu.lock();
    }
private:
    Books books;
    std::unordered_map<long,Order*> allOrders;
    SpinLock mu;
    long nextID();
    long insertOrder(std::string instrument,F price,int quantity,Side side,std::string orderId);
    ExchangeListener& listener;
    static const int BLOCK_LEN = 65536;
    static const int ORDER_LEN = sizeof(Order);
    uint8_t * currentBlock = (uint8_t*)malloc(BLOCK_LEN);
    int blockUsed = 0;
    void * allocateOrder() {
        /** since all orders have a reference maintained to them, use an efficient bump allocator.
            This currently leaks even if the Exchange instance is destroyed. TODO track allocated
            blocks on a list to free in destructor. */
        if(BLOCK_LEN-blockUsed < ORDER_LEN) {
            currentBlock = (uint8_t*)malloc(BLOCK_LEN);
            blockUsed=0;
        }
        void * ptr = currentBlock + blockUsed;
        blockUsed+=ORDER_LEN;
        return ptr;
    }
};