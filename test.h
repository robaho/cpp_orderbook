#pragma once

#include "exchange.h"

class TestExchange : public Exchange {
public:
    TestExchange() {}
    TestExchange(ExchangeListener& listener) : Exchange(listener){}
    auto buy(F price,int quantity,std::string orderId) {
        return Exchange::buy("dummy",price,quantity,orderId);
    }
    auto sell(F price,int quantity,std::string orderId) {
        return Exchange::sell("dummy",price,quantity,orderId);
    }
    auto marketBuy(int quantity,std::string orderId) {
        return Exchange::marketBuy("dummy",quantity,orderId);
    }
    auto marketSell(int quantity,std::string orderId) {
        return Exchange::marketSell("dummy",quantity,orderId);
    }
    auto book() {
        return Exchange::book("dummy");
    }
    auto bidCount() { return book().bids.size(); }
    auto askCount() { return book().asks.size(); }
    int bidIndex(long exchangeId) { auto b = book(); return std::find(b.bidOrderIds.begin(),b.bidOrderIds.end(),exchangeId) - b.bidOrderIds.begin(); }
    int askIndex(long exchangeId) { auto b = book(); return std::find(b.askOrderIds.begin(),b.askOrderIds.end(),exchangeId) - b.askOrderIds.begin(); }
};

static const std::string dummy_instrument = "dummy";
static const std::string dummy_oid = "";

class TestOrder : public Order {
public:
    TestOrder(long id,F price,int quantity,Side side) : Order(dummy_oid,dummy_instrument,price,quantity,side,id) {}
    TestOrder(std::string orderId,long id,F price,int quantity,Side side) : Order(orderId,dummy_instrument,price,quantity,side,id) {}
};
