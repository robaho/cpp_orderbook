#pragma once

#include <chrono>
#include <list>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map>
#include <deque>

#include "order.h"
#include "orderlist.h"
#include "fixed.h"

struct price_compare {
    explicit price_compare(bool ascending) : ascending(ascending) {}
    template<class T, class U>
    bool operator()(const T& t, const U& u) const {
        if(ascending) { return t < u; }
        else { return t > u; }
    }
    const bool ascending;
};

class PriceLevels {
friend class OrderBook;
private:
    void insertOrder(Order *order) {
        auto itr = levels.find(order->price);
        if(itr==levels.end()) {
            levels[order->price] = OrderList{};
        }
        levels[order->price].pushback(order);
    }
    void removeOrder(Order *order) {
        auto itr = levels.find(order->price);
        if(itr==levels.end()) throw new std::runtime_error("price level for order does not exist");
        levels[order->price].remove(order);
        if(levels[order->price].front()==nullptr) {
            levels.erase(order->price);
        }
    }
    std::map<F,OrderList,price_compare> levels;
public:
    PriceLevels(bool ascendingPrices) : levels(price_compare(ascendingPrices)) {}
    bool empty() { return levels.empty(); }
    Order* front() { return levels.begin()->second.front();}
    int size() { return levels.size(); }
};

struct Trade {
friend class OrderBook;
private:
    Trade(F price,int quantity,const Order& aggressor,const Order& opposite) : price(price), quantity(quantity), aggressor(aggressor), opposite(opposite){}
public:
    const F price;
    const int quantity;
    const Order& aggressor;
    const Order& opposite;
};

typedef void (*TradeReceiver)(Trade);

class OrderBookListener {
public:
    virtual void onOrder(const Order& order){}
    virtual void onTrade(const Trade& trade){}
};

struct BookLevel {
    const F price;
    const int quantity;
    BookLevel(F price,int quantity) : price(price), quantity(quantity){}
};

struct Book {
    std::vector<BookLevel> bids;
    std::vector<long> bidOrderIds;
    std::vector<BookLevel> asks;
    std::vector<long> askOrderIds;
};

class Exchange;

class OrderBook {
private:
    std::mutex mu;
    PriceLevels bids = PriceLevels(false);    
    PriceLevels asks = PriceLevels(true);
    OrderBookListener& listener;
    void matchOrders(Side aggressorSide);
public:
    OrderBook(OrderBookListener& listener) : listener(listener){}
    void insertOrder(Order* order);
    int cancelOrder(Order *order);
    const Book book();
    const Order getOrder(Order *order);
};