#pragma once

#include <chrono>
#include <list>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map>
#include <deque>
#include <list>

#include "order.h"
#include "orderlist.h"
#include "fixed.h"
#include "spinlock.h"
#include "pricelevels.h"

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
    static const int BLOCK_LEN = 65536;
    static const int ORDER_LEN = sizeof(Order);
    uint8_t * currentBlock = (uint8_t*)malloc(BLOCK_LEN);
    int blockUsed = 0;
    std::list<void*> blocks;
public:
    const std::string instrument;
    OrderBook(const std::string &instrument,OrderBookListener& listener) : listener(listener), instrument(instrument){}
    ~OrderBook() {
        for(auto ptr : blocks) {
            free(ptr);
        }
        free(currentBlock);
    }
    void insertOrder(Order* order);
    int cancelOrder(Order *order);
    const Book book();
    const Order getOrder(Order *order);
    Guard lock() {
        return Guard(mu);
    }
    void * allocateOrder() {
        /** since all orders have a reference maintained to them, use an efficient bump allocator.
            This currently leaks even if the Exchange instance is destroyed. TODO track allocated
            blocks on a list to free in destructor. */
        if(BLOCK_LEN-blockUsed < ORDER_LEN) {
            blocks.push_back(currentBlock);
            currentBlock = (uint8_t*)malloc(BLOCK_LEN);
            blockUsed=0;
        }
        void * ptr = currentBlock + blockUsed;
        blockUsed+=ORDER_LEN;
        return ptr;
    }
};