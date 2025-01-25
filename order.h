#pragma once

#include <chrono>
#include <cfloat>

#include "fixed.h"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

#define epoch() std::chrono::system_clock::now().time_since_epoch()

typedef Fixed<7> F;

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
public:
    enum Side { BUY, SELL};

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
    // sessionId cannot be a reference since the Order will out-live the Session
    const std::string _sessionId;
    std::string _orderId;

    F _price;
    int _quantity;
    int _cumQty=0;
    F _avgPrice=0;

    bool _isQuote = false;
    
    void fill(int quantity,F price) { 
        remaining -= quantity; filled += quantity;  
        _avgPrice = (_avgPrice*_cumQty + price*quantity) / (_cumQty + quantity);
        _cumQty += quantity; 
    }
    void cancel() { remaining = 0; }
    bool isMarket() { return _price == DBL_MAX || _price == -DBL_MAX; } // could add "type" property, but not necessary for only limit and market orders
protected:
    // protected to allow testcase
    Order(const std::string& sessionId,const std::string &orderId,const std::string &instrument,F price,int quantity,Order::Side side,long exchangeId) : timeSubmitted(epoch()), remaining(quantity),
     _sessionId(sessionId), _orderId(orderId), _price(price), _quantity(quantity), instrument(instrument), exchangeId(exchangeId), side(side) {}
public:

    const std::string& sessionId() const { return _sessionId; }
    const std::string& orderId() const { return _orderId; }
    const std::string &instrument; 
    const long exchangeId;
    const Side side;

    bool isOnList() const {
        return node.order!=nullptr;
    }

    bool isQuote() const{
        return _isQuote;
    }

    F price() const { return _price; }
    int quantity() const { return _quantity; }

    int remainingQuantity() const {
        return remaining;
    }
    int filledQuantity() const {
        return filled;
    }
    int cumulativeQuantity() const {
        return _cumQty;
    }
    F averagePrice() const {
        return _avgPrice;
    }
    bool isCancelled() const {
        return remaining==0 && filled!=_quantity;
    }
    bool isFilled() const {
        return remaining==0 && filled==_quantity;
    }
    bool isPartiallyFilled() const {
        return remaining==0 && filled>0;
    }
    bool isActive() const {
        return remaining>0;
    }
};
