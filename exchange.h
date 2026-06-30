#pragma once

#include <string>

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
    long buy(const std::string& sessionId,const std::string_view& instrument,F price,int quantity,const std::string_view& orderId);
    /** submit market buy order. returns exchange order id. If the order cannot be filled, the rest is cancelled */
    long marketBuy(const std::string& sessionId,const std::string_view& instrument,int quantity,const std::string_view& orderId) {
        return buy(sessionId,instrument,DBL_MAX,quantity,orderId);
    }
    /** submit limit sell order. returns exchange order id */
    long sell(const std::string& sessionId,const std::string_view& instrument,F price,int quantity,const std::string_view& orderId);
    /** submit market sell order. returns exchange order id. If the order cannot be filled, the rest is cancelled */
    long marketSell(const std::string& sessionId,const std::string_view& instrument,int quantity,const std::string_view& orderId) {
        return sell(sessionId,instrument,-DBL_MAX,quantity,orderId);
    }
    void quote(const std::string& sessionId,const std::string_view& instrument,F bidPrice,int bidQuantity,F askPrice,int askQuantity,const std::string_view& quoteId);
    // returns 0 if cancelled, else error
    int cancel(long exchangeId,const std::string& sessionId);
    const Book book(const std::string& instrument);
    const Order getOrder(long exchangeId);
    void onOrder(const Order& order) override {
        listener.onOrder(order);
    }
    void onTrade(const Trade& trade) override {
        listener.onTrade(trade);
    }
    Guard lock() {
        return Guard(mu);
    }
    std::vector<std::string> instruments() {
        return books.instruments();
    }
    std::vector<const Order*> orders() {
        return allOrders.all();
    }
private:
    BookMap books;
    OrderMap allOrders;
    SpinLock mu;
    long nextID();
    long insertOrder(const std::string& sessionId,const std::string_view& instrument,F price,int quantity,Order::Side side,const std::string_view& orderId);
    ExchangeListener& listener;
};