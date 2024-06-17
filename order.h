#pragma once

#include <chrono>
#include <cfloat>

#include "fixed.h"


typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

#define epoch() std::chrono::system_clock::now().time_since_epoch()

typedef Fixed<7> F;

enum Side { BUY, SELL};

class Exchange;

struct Order {
friend class OrderBook;
friend class OrderList;
friend class Exchange;
private:
    const TimePoint timeSubmitted;

    int remaining;
    int filled=0;
    
    Order(std::string orderId,std::string& instrument,F price,int quantity,Side side,long exchangeId) : orderId(orderId), instrument(instrument), exchangeId(exchangeId) , price(price), quantity(quantity), side(side), remaining(quantity), timeSubmitted(epoch()){}
    void fill(int quantity) { remaining -= quantity; filled += quantity; }
    void cancel() { remaining = 0; }
    bool isMarket() { return price == DBL_MAX || price == -DBL_MAX; } // could add "type" property, but not necessary for only limit and market orders
public:
    const std::string orderId;
    const std::string instrument; 
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
