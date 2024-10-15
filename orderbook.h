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
#include "spinlock.h"

struct price_compare {
    explicit price_compare(bool ascending) : ascending(ascending) {}
    template<class T, class U>
    inline bool operator()(const T& t, const U& u) const {
        return (ascending) ? t->price < u : t->price > u;
    }
    const bool ascending;
};

class PriceLevels {
friend class OrderBook;
private:
    void insertOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price, cmpFn);
        OrderList *list;
        if(itr==levels.end() || (*itr)->price!=order->price) {
            list = new OrderList(order->price);
            levels.insert(itr,list);
        } else list = *itr;
        list->pushback(order);
    }
    void removeOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price, cmpFn);
        if(itr==levels.end() || (*itr)->price!=order->price) throw new std::runtime_error("price level for order does not exist");
        OrderList *list = *itr;
        list->remove(order);
        if(list->front()==nullptr) {
            levels.erase(itr);
            free(list);
        }
    }
    const price_compare cmpFn;
    std::deque<OrderList*> levels;
public:
    PriceLevels(bool ascendingPrices) : cmpFn(price_compare(ascendingPrices)) {}
    bool empty() { return levels.empty(); }
    Order* front() { return levels.front()->front();}
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
    virtual void onOrder(const Order& order) {}
    virtual void onTrade(const Trade& trade) {}
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

/** OrderBook instances are single threaded and must be externally synchronized using mu or lock() */
class OrderBook {
private:
    SpinLock mu;
    PriceLevels bids = PriceLevels(false);    
    PriceLevels asks = PriceLevels(true);
    OrderBookListener& listener;
    void matchOrders(Side aggressorSide);
public:
    const std::string instrument;
    OrderBook(const std::string &instrument,OrderBookListener& listener) : listener(listener), instrument(instrument){}
    void insertOrder(Order* order);
    int cancelOrder(Order *order);
    const Book book();
    const Order getOrder(Order *order);
    Guard lock() {
        return mu.lock();
    }
};