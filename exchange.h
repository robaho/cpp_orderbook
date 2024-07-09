#pragma once

#include <string>
#include <unordered_map>

#include "orderbook.h"

struct ExchangeListener {
    /** callback when order properties change */
    virtual void onOrder(const Order& order){};
    /** callback when trade occurs */
    virtual void onTrade(const Trade& trade){};
};

class Exchange {
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
    const Book book(std::string instrument);
    const Order getOrder(long exchangeId);
protected:
    OrderBook* orderBook(std::string symbol) { return books[symbol]; }
private:
    std::unordered_map<std::string,OrderBook*> books;
    std::map<long,Order*> allOrders;
    std::mutex mu;
    long nextID();
    long insertOrder(std::string instrument,F price,int quantity,Side side,std::string orderId);
    ExchangeListener& listener;
};