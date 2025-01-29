#pragma once

#include <list>
#include <vector>
#include <map>
#include <list>
#include <iostream>

#include "order.h"
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
    const long execId = epoch().count();
};

typedef void (*TradeReceiver)(Trade);

class OrderBookListener {
public:
    virtual void onOrder(const Order& order) {}
    virtual void onTrade(const Trade& trade) {}
};

struct BookLevel {
    F price;
    int quantity;
};

struct Book {
    std::vector<BookLevel> bids;
    std::vector<long> bidOrderIds;
    std::vector<BookLevel> asks;
    std::vector<long> askOrderIds;
};

inline std::ostream& operator<<(std::ostream& os, const Book& book) {
    bool first=true;
    for (auto side : {book.asks,book.bids}) {
        for(auto level : side) {
            os << level.price << " " << level.quantity << "\n";
        }
        if(first) { os << "----------\n"; first=false; }
    }
    return os;
}

// map of Session+QuoteId to the associated orders, or null if no quote on that side
struct QuoteOrders {
    Order* bid=nullptr;
    Order* ask=nullptr;
};

struct SessionQuoteId {
    const std::string sessionId;
    const std::string quoteId;
    SessionQuoteId(const std::string& sessionId,const std::string_view& quoteId) : sessionId(sessionId), quoteId(quoteId){}
    bool operator<(const SessionQuoteId& other) const {
        return sessionId<other.sessionId || (sessionId==other.sessionId && quoteId<other.quoteId);
    }
    bool operator==(const SessionQuoteId& other) const {
        return sessionId==other.sessionId && quoteId==other.quoteId;
    }
};

inline std::ostream& operator<<(std::ostream& os, const SessionQuoteId& id) {
    return os << "[" << id.sessionId << ":" << id.quoteId << "]";
}

class Exchange;

/** OrderBook instances are single threaded and must be externally synchronized using mu or lock() */
class OrderBook {
private:
    SpinLock mu;
    PriceLevels bids = PriceLevels(false);    
    PriceLevels asks = PriceLevels(true);
    OrderBookListener& listener;
    void matchOrders(Order::Side aggressorSide);
    static const int BLOCK_LEN = 65536;
    static const int ORDER_LEN = sizeof(Order);
    uint8_t * currentBlock = (uint8_t*)malloc(BLOCK_LEN);
    int blockUsed = 0;
    std::list<void*> blocks;
    std::map<SessionQuoteId,QuoteOrders> quotes;
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

    QuoteOrders getQuotes(const std::string& sessionId, const std::string& quoteId, std::function<QuoteOrders()> createOrders);
    void quote(const QuoteOrders& quotes,F bidPrice,int bidQuantity,F askPrice,int askQuantity);

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