#pragma once

#include <chrono>
#include <list>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map>
#include <cfloat>

#include "fixed.h"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

#define epoch() std::chrono::system_clock::now().time_since_epoch()

typedef Fixed<7> F;

enum Side { BUY, SELL};

struct Order {
friend class OrderBook;
friend class OrderList;
private:
    const TimePoint timeSubmitted;

    int remaining;
    int filled=0;
    
    Order(F price,int quantity,Side side,long id) : id(id) , price(price), quantity(quantity), side(side), remaining(quantity), timeSubmitted(epoch()){}
    void fill(int quantity) { remaining -= quantity; filled += quantity; }
    void cancel() { remaining = 0; }
    bool isMarket() { return price == DBL_MAX || price == -DBL_MAX; } // could add "type" property, but not necessary for only limit and market orders
public:
    const long id;
    const F price;
    const int quantity;
    const Side side;

    int remainingQuantity() const {
        return remaining;
    }
    int filledQuantity() const {
        return filled;
    }
    bool isCancelled() {
        return remaining==0 && filled!=quantity;
    }
    bool isFilled() {
        return remaining==0 && filled==quantity;
    }
    bool isPartiallyFilled() {
        return remaining==0 && filled>0;
    }
    bool isActive() {
        return remaining>0;
    }
};

class OrderList : private std::vector<Order*> {
friend class OrderBook;
private:
    void insertOrder(Order *order,bool ascending);
public:
    int indexOf(long id) {
        for(auto itr = begin();itr!=end();itr++) {
            if((*itr)->id == id) {
                return itr - begin();
            }
        }
        return -1;
    }
    int size() {
        return vector::size();
    }
    Order* frontOrder() {
        return vector::front();
    }
};

struct Trade {
friend class OrderBook;
private:
    Trade(F price,int quantity,long aggressor,long opposite) : price(price), quantity(quantity), aggressor(aggressor), opposite(opposite){}
public:
    const F price;
    const int quantity;
    const long aggressor;
    const long opposite;
};

typedef void (*TradeReceiver)(Trade);

class Listener {
public:
    virtual void onOrder(const Order& order){}
    virtual void onTrade(const Trade& trade){}
};

struct Book {
    std::vector<Order> bids;
    std::vector<Order> asks;
};

class OrderBook {
private:
    std::mutex mu;
    OrderList bids;    
    OrderList asks;
    long nextID();
    Listener& listener;
    void matchOrders(Side aggressorSide);
    std::map<long,Order*> allOrders;
    std::map<long,Trade*> allTrades;
    static Listener& dummy() {
        static Listener dummy;
        return dummy;
    }
public:
    OrderBook() : OrderBook(dummy()){}
    OrderBook(Listener& listener) : listener(listener){}
    long buy(F price,int quantity);
    long sell(F price, int quantity);
    long marketBuy(int quantity) { return buy(DBL_MAX,quantity); }
    long marketSell(int quantity) { return sell(-DBL_MAX,quantity); }
    /** internal testing method */
    OrderList bidList() const { return bids; }
    /** internal testing method */
    OrderList askList() const { return asks; }
    Book book();
    const Order getOrder(long id);
    /**
     * @brief cancel an order
     * 
     * @param id the order id
     * @return int returns 0 on success, -1 if the order has already been filled or cancelled
     */
    int cancel(long id);
};