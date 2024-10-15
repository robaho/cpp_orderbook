#include "exchange.h"
#include "orderbook.h"
#include <mutex>
#include <stdexcept>

#define LOCK_EXCHANGE() std::lock_guard<std::mutex> lock(mu)
#define UNLOCK_EXCHANGE() lock.

static ExchangeListener dummy;

Exchange::Exchange(ExchangeListener& listener) : listener(listener) {}
Exchange::Exchange() : listener(dummy) {}

const Order Exchange::getOrder(long exchangeId) {
    OrderBook* book;
    Order* order;

    {
        auto guard = lock();
        order = allOrders.get(exchangeId);
    }
    if(!order) throw std::runtime_error("invalid exchange order id");
    book = books.get(order->instrument);
    if(!book) throw std::runtime_error("invalid exchange order id");
    auto bookGuard = book->lock();
    return book->getOrder(order);
}

const Book Exchange::book(const std::string& instrument) {
    OrderBook* book;

    book = books.getOrCreate(instrument,*this);
    auto bookGuard = book->lock();
    return book->book();
}

int Exchange::cancel(long exchangeId) {
    OrderBook* book;
    Order* order;

    {
        auto guard = lock();
        order = allOrders.get(exchangeId);
    }
    if(!order) throw std::runtime_error("invalid exchange order id");
    book = books.get(order->instrument);
    if(!book) throw std::runtime_error("invalid exchange order id");

    auto bookGuard = book->lock();
    return book->cancelOrder(order);
}

long Exchange::insertOrder(std::string instrument,F price,int quantity,Side side,std::string orderId) {
    OrderBook *book = books.getOrCreate(instrument,*this);
    auto bookGuard = book->lock();
    long id = nextID();
    Order *order = new (book->allocateOrder()) Order(orderId,book->instrument,price,quantity,side,id);
    {
        auto guard = lock();
        allOrders.add(order);
    }

    book->insertOrder(order);
    return id;
}

long Exchange::buy(std::string instrument,F price,int quantity,std::string orderId) {
    return insertOrder(instrument,price,quantity,BUY,orderId);
}

long Exchange::sell(std::string instrument,F price,int quantity,std::string orderId) {
    return insertOrder(instrument,price,quantity,SELL,orderId);
}

long Exchange::nextID() {
    static std::atomic<long> id = 0;
    return ++id;
}
