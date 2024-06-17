#include "orderbook.h"
#include "order.h"
#include <mutex>
#include <sys/param.h>

#define LOCK_BOOK() std::lock_guard<std::mutex> lock(mu)

void OrderList::insertOrder(Order *order) {
    auto lessFn = [this](Order *a,Order *b) {
        return ascending ? a->price < b->price : b->price < a->price;
    };
    auto itr = begin();
    for(;itr!=end() && !lessFn(order,*itr);itr++);
    if(itr==end()) {
        push_back(order);
    } else {
        insert(itr,order);
    }
}

void OrderBook::insertOrder(Order *order) {
    LOCK_BOOK();
    auto list = order->side == BUY ? &bids : &asks;
    list->insertOrder(order);
    listener.onOrder(*order);
    matchOrders(order->side);
}

void OrderBook::matchOrders(Side aggressorSide) {
    while(!bids.empty() && !asks.empty()) {
        auto bid = bids.front();
        auto ask = asks.front();

        if(bid->price >= ask->price) {
            int qty = MIN(bid->remaining,ask->remaining);
            F price = MIN(bid->price,ask->price);

            Order* aggressor = aggressorSide==BUY ? bid : ask;
            Order* opposite = aggressorSide==BUY ? ask : bid;

            bid->fill(qty);
            ask->fill(qty);

            auto trade = new Trade(price,qty,*aggressor,*opposite);

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

int OrderBook::cancelOrder(Order *order) {
    LOCK_BOOK();
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

const Book OrderBook::book() {
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
    return book;
}

const Order OrderBook::getOrder(Order* order) {
    LOCK_BOOK();
    return *order;
}
