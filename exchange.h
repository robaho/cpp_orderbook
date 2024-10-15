#pragma once

#include <string>
#include <unordered_map>

#include "order.h"
#include "orderbook.h"
#include "bookmap.h"
#include "spinlock.h"
#include "ordermap.h"

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
    BookMap books;
    OrderMap allOrders;
    SpinLock mu;
    long nextID();
    long insertOrder(std::string instrument,F price,int quantity,Side side,std::string orderId);
    ExchangeListener& listener;
};