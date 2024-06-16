#include "orderbook.h"
#include <mutex>
#include <sys/param.h>

#define LOCK_BOOK() std::lock_guard<std::mutex> lock(mu)

long OrderBook::buy(F price,int quantity) {
    LOCK_BOOK();
    long id = nextID();
    auto newOrder = new Order(price,quantity,BUY,id);
    bids.insertOrder(newOrder,false);
    allOrders[id]=newOrder;
    listener.onOrder(*newOrder);
    matchOrders(BUY);
    return id;
}

long OrderBook::sell(F price,int quantity) {
    LOCK_BOOK();
    long id = nextID();
    auto newOrder = new Order(price,quantity,SELL,id);
    asks.insertOrder(newOrder,true);
    allOrders[id]=newOrder;
    listener.onOrder(*newOrder);
    matchOrders(SELL);
    return id;
}

long OrderBook::nextID() {
    static long id = 0;
    return ++id;
}

void OrderList::insertOrder(Order *order,bool ascendingPrice) {
    auto lessFn = [ascendingPrice](Order *a,Order *b) {
        return ascendingPrice ? a->price < b->price : b->price < a->price;
    };
    auto itr = begin();
    for(;itr!=end() && !lessFn(order,*itr);itr++);
    if(itr==end()) {
        push_back(order);
    } else {
        insert(itr,order);
    }
}

void OrderBook::matchOrders(Side aggressorSide) {

    while(!bids.empty() && !asks.empty()) {
        auto bid = bids.front();
        auto ask = asks.front();

        if(bid->price >= ask->price) {
            int qty = MIN(bid->remaining,ask->remaining);
            F price = MIN(bid->price,ask->price);
            long aggressor = aggressorSide==BUY ? bid->id : ask->id;
            long opposite = aggressorSide==BUY ? ask->id : bid->id;
            auto trade = new Trade(price,qty,aggressor,opposite);
            bid->fill(qty);
            ask->fill(qty);
            if(bid->remaining==0) {
                bids.erase(bids.begin());
            }
            if(ask->remaining==0) {
                asks.erase(asks.begin());
            }
            listener.onOrder(*bid);
            listener.onOrder(*ask);
            listener.onTrade(*trade);
        }
    }
    // cancel remaining market order
    // TODO support convert to limit order
    auto orders = aggressorSide==BUY ? &bids : &asks;
    if(!orders->empty()) {
        auto order = orders->front();
        if(order->isMarket()) { 
            order->cancel();
            orders->erase(orders->begin());
            listener.onOrder(*order);
        }
    }
}

const Order OrderBook::getOrder(long id) {
    LOCK_BOOK();
    auto order = allOrders[id];
    if(!order) throw std::runtime_error("invalid order id");
    return *order;
}

int OrderBook::cancel(long id) {
    LOCK_BOOK();
    auto order = allOrders[id];
    if(!order) throw std::runtime_error("invalid order id");
    if(order->remaining>0) {
        order->cancel();
        auto orders = order->side == BUY ? &bids : &asks;
        auto itr = std::find(orders->begin(),orders->end(),order);
        orders->erase(itr);
        listener.onOrder(*order);
        return 0;
    } else {
        return -1;
    }
}

Book OrderBook::book() {
    LOCK_BOOK();
    Book book;
    auto snap = [](const OrderList& src,std::vector<Order> &dst) {
        for(auto itr=src.begin();itr<src.end();itr++) {
            auto order = *itr;
            dst.push_back(*order);
        }
    };
    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());
    snap(bids,book.bids);
    snap(asks,book.asks);
    return std::move(book);
}