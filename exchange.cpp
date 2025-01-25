#include "exchange.h"
#include "orderbook.h"
#include <stdexcept>
#include <string>

#define LOCK_EXCHANGE() std::lock_guard<std::mutex> lock(mu)
#define UNLOCK_EXCHANGE() lock.

static ExchangeListener dummy;

Exchange::Exchange(ExchangeListener& listener) : listener(listener) {}
Exchange::Exchange() : listener(dummy) {}

const Order Exchange::getOrder(long exchangeId) {
    OrderBook* book;
    Order* order = allOrders.get(exchangeId);
    if(!order) throw std::runtime_error("invalid exchange order id "+std::to_string(exchangeId));
    book = books.get(order->instrument);
    if(!book) throw std::runtime_error("missing book for order id"+std::to_string(exchangeId));
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
    Order* order = allOrders.get(exchangeId);
    if(!order) throw std::runtime_error("invalid exchange order id "+std::to_string(exchangeId));
    book = books.get(order->instrument);
    if(!book) throw std::runtime_error("missing book for order id"+std::to_string(exchangeId));

    auto bookGuard = book->lock();
    return book->cancelOrder(order);
}

long Exchange::insertOrder(const std::string& sessionId,const std::string_view& instrument,F price,int quantity,Order::Side side,const std::string_view& orderId) {
    OrderBook *book = books.getOrCreate(instrument,*this);
    auto bookGuard = book->lock();
    long id = nextID();
    Order *order = new (book->allocateOrder()) Order(sessionId,std::string(orderId),book->instrument,price,quantity,side,id);
    allOrders.add(order);

    book->insertOrder(order);
    return id;
}

long Exchange::buy(const std::string& sessionId,const std::string_view& instrument,F price,int quantity,const std::string_view& orderId) {
    return insertOrder(sessionId,instrument,price,quantity,Order::BUY,orderId);
}

long Exchange::sell(const std::string& sessionId,const std::string_view& instrument,F price,int quantity,const std::string_view& orderId) {
    return insertOrder(sessionId,instrument,price,quantity,Order::SELL,orderId);
}

void Exchange::quote(const std::string& sessionId,const std::string_view& instrument,F bidPrice,int bidQuantity,F askPrice,int askQuantity,const std::string_view& quoteId) {
    OrderBook *book = books.getOrCreate(instrument,*this);
    auto bookGuard = book->lock();
    auto orders = book->getQuotes(sessionId,std::string(quoteId),[&]() -> QuoteOrders{
        auto bid = new (book->allocateOrder()) Order(sessionId,std::string(quoteId),book->instrument,bidPrice,bidQuantity,Order::BUY,nextID());
        auto ask = new (book->allocateOrder()) Order(sessionId,std::string(quoteId),book->instrument,askPrice,askQuantity,Order::SELL,nextID());
        allOrders.add(bid);
        allOrders.add(ask);
        return {bid,ask};
    });
    book->quote(orders,bidPrice,bidQuantity,askPrice,askQuantity);
}

long Exchange::nextID() {
    static std::atomic<long> id = 0;
    return ++id;
}
