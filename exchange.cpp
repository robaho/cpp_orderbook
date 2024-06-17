#include "exchange.h"
#include "orderbook.h"
#include <stdexcept>

#define LOCK_EXCHANGE() std::lock_guard<std::mutex> lock(mu)

static ExchangeListener dummy;

Exchange::Exchange(ExchangeListener& listener) : listener(listener) {}
Exchange::Exchange() : listener(dummy) {}

struct MyOrderBookListener : OrderBookListener {
    ExchangeListener& el;
    MyOrderBookListener(ExchangeListener& el) : el(el){}
    void onOrder(const Order& order) override {
        el.onOrder(order);
    }
    void onTrade(const Trade& trade) override {
        el.onTrade(trade);
    }
};

const Order Exchange::getOrder(long exchangeId) {
    OrderBook* book;
    Order* order;

    {
        LOCK_EXCHANGE();

        order = allOrders[exchangeId];
        if(!order) throw std::runtime_error("invalid exchange order id");
        book = books[order->instrument];
        assert(book!=nullptr);
    }

    return book->getOrder(order);
}

const Book Exchange::book(std::string instrument) {
    OrderBook* book;
    {
        LOCK_EXCHANGE();
        book = books[instrument];
    }

    return book->book();
}

int Exchange::cancel(long exchangeId) {
    OrderBook* book;
    Order* order;

    {
        LOCK_EXCHANGE();

        order = allOrders[exchangeId];
        if(!order) throw std::runtime_error("invalid exchange order id");
        book = books[order->instrument];
        assert(book!=nullptr);
    }

    return book->cancelOrder(order);
}

long Exchange::insertOrder(std::string instrument,F price,int quantity,Side side,std::string orderId) {
    long id;
    OrderBook *book;
    Order *order;
    {
        LOCK_EXCHANGE();
        id = nextID();
        book = books[instrument];
        if(!book) {
            auto obl = new MyOrderBookListener(listener);
            book = new OrderBook(*obl);
            books[instrument] = book;
        }
        order = new Order(orderId,instrument,price,quantity,side,id);
        allOrders[id]=order;
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
    static long id = 0;
    return ++id;
}
