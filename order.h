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
    const std::string& _sessionId;
    std::string _orderId;

    F _price;
    int _quantity;

    bool isQuote = false;
    
    void fill(int quantity) { remaining -= quantity; filled += quantity; }
    void cancel() { remaining = 0; }
    bool isMarket() { return _price == DBL_MAX || _price == -DBL_MAX; } // could add "type" property, but not necessary for only limit and market orders
protected:
    // protected to allow testcase
    Order(const std::string& sessionId,const std::string &orderId,const std::string &instrument,F price,int quantity,Side side,long exchangeId) : timeSubmitted(epoch()), remaining(quantity),
     _sessionId(sessionId), _orderId(orderId), _price(price), _quantity(quantity), instrument(instrument), exchangeId(exchangeId), side(side) {}
public:
    const std::string& sessionId() { return _sessionId; }
    const std::string& orderId() { return _orderId; }
    const std::string &instrument; 
    const long exchangeId;
    const Side side;

    bool isOnList() {
        return node.order!=nullptr;
    }

    F price() const { return _price; }
    int quantity() const { return _quantity; }

    int remainingQuantity() const {
        return remaining;
    }
    int filledQuantity() const {
        return filled;
    }
    bool isCancelled() {
        return remaining==0 && filled!=_quantity;
    }
    bool isFilled() {
        return remaining==0 && filled==_quantity;
    }
    bool isPartiallyFilled() {
        return remaining==0 && filled>0;
    }
    bool isActive() {
        return remaining>0;
    }
};
