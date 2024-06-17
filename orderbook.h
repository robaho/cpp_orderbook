#pragma once

#include <chrono>
#include <list>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map>

#include "order.h"

class OrderList : private std::vector<Order*> {
friend class OrderBook;
private:
    void insertOrder(Order *order);
    const bool ascending;
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
    int size() {
        return vector::size();
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

struct Book {
    std::vector<Order> bids;
    std::vector<Order> asks;
    static int indexOf(std::vector<Order> &list,long exchangeId) {
        auto itr = std::find_if(list.begin(),list.end(),[exchangeId](const Order& order){return order.exchangeId==exchangeId;});
        return itr==list.end() ? -1 : itr-list.begin();
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