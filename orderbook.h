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

class OrderList : private std::deque<Order*> {
friend class OrderBook;
private:
    void insertOrder(Order *order);
    const bool ascending;
    bool lessFn(Order *a,Order *b) {
        if(a->price==b->price) return a->timeSubmitted < b->timeSubmitted;
        return ascending ? a->price < b->price : b->price < a->price;
    };

public:
    OrderList(bool ascendingPrices) : ascending(ascendingPrices){}
    int indexOf(long id) {
        for(auto itr = begin();itr!=end();itr++) {
            if((*itr)->exchangeId == id) {
                return itr - begin();
            }
        }
        return -1;
    }
    auto find(Order *order) {
        auto cmp =[this](Order *a,Order*b) {
            return lessFn(a,b);
        };
        auto itrs = std::equal_range(begin(),end(),order,cmp);
        for(auto itr=itrs.first;itr!=itrs.second;itr++) {
            if((*itr)==order) return itr;
        }
        return end();
    }
    int size() {
        return deque::size();
    }
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

typedef std::pair<F,int> BookLevel;

/** consolidated book levels by price and total quantity quantity */
struct BookLevels {
    std::vector<BookLevel> bids;
    std::vector<BookLevel> asks;
};

struct Book {
    std::vector<Order> bids;
    std::vector<Order> asks;
    static int indexOf(std::vector<Order> &list,long exchangeId) {
        auto itr = std::find_if(list.begin(),list.end(),[exchangeId](const Order& order){return order.exchangeId==exchangeId;});
        return itr==list.end() ? -1 : itr-list.begin();
    }
    BookLevels levels() const {
        BookLevels levels;
        auto process = [](std::vector<BookLevel> &levels,const std::vector<Order> &orders) {
            auto itr = orders.begin();
            for(;itr!=orders.end();) {
                int total = itr->quantity;
                F price = itr->price;
                while(++itr!=orders.end() && itr->price==price) {
                    total+=itr->quantity;
                }
                levels.push_back(std::pair(price,total));
            }
        };
        process(levels.bids,bids);
        return levels;
    }
};

class Exchange;

class OrderBook {
private:
    std::mutex mu;
    OrderList bids = OrderList(false);    
    OrderList asks = OrderList(true);
    OrderBookListener& listener;
    void matchOrders(Side aggressorSide);
public:
    OrderBook(OrderBookListener& listener) : listener(listener){}
    void insertOrder(Order* order);
    int cancelOrder(Order *order);
    const Book book();
    const Order getOrder(Order *order);
};