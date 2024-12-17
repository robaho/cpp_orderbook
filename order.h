#pragma once

#include <chrono>
#include <cfloat>

#include "fixed.h"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

#define epoch() std::chrono::system_clock::now().time_since_epoch()

typedef Fixed<7> F;

enum Side { BUY, SELL};

class Exchange;
class OrderList;
class OrderMap;

struct Order;

class Node {
friend class OrderList;
friend struct Order;
private:
    Node* prev=nullptr;
    Node* next=nullptr;
    /** order is non-null if enqueued on an OrderList */
    Order *order=nullptr;
};

struct OrderId {
// used to avoid dynamic memory in std::string, even though SSO (small string optimization) usually avoids it
private:
    char buffer[64];
public:
    OrderId(const std::string& oid) {
        auto len = oid.copy(buffer, 64);
        buffer[len]=0;
    }
    operator const char *() const { return buffer; }
};

struct Order {
friend class OrderBook;
friend class OrderList;
friend class OrderMap;
friend class Exchange;
private:
    /** used to enqueue Order in OrderMap */
    Order *next = nullptr;
    /** holds Node in OrderList for quick removal */
    Node node;
    const TimePoint timeSubmitted;

    int remaining;
    int filled=0;
    OrderId _orderId;
    
    void fill(int quantity) { remaining -= quantity; filled += quantity; }
    void cancel() { remaining = 0; }
    bool isMarket() { return price == DBL_MAX || price == -DBL_MAX; } // could add "type" property, but not necessary for only limit and market orders
protected:
    // protected to allow testcase
    Order(const std::string &orderId,const std::string &instrument,F price,int quantity,Side side,long exchangeId) : timeSubmitted(epoch()), remaining(quantity),
     _orderId(orderId), orderId(_orderId), instrument(instrument), exchangeId(exchangeId) , price(price), quantity(quantity), side(side) {}
public:
    const char * const orderId;
    const std::string &instrument; 
    const long exchangeId;
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
