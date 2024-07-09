#include "orderbook.h"
#include "order.h"
#include <algorithm>
#include <mutex>
#include <sys/param.h>

#define LOCK_BOOK() std::lock_guard<std::mutex> lock(mu)

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
                bids.removeOrder(bid);
            }
            if(ask->remaining==0) {
                asks.removeOrder(ask);
            }
            listener.onOrder(*bid);
            listener.onOrder(*ask);
            listener.onTrade(*trade);
        } else {
            break;
        }
    }
    // cancel remaining market order
    // TODO support convert to limit order
    auto orders = aggressorSide==BUY ? &bids : &asks;
    if(!orders->empty()) {
        auto order = orders->front();
        if(order->isMarket()) { 
            order->cancel();
            orders->removeOrder(order);
            listener.onOrder(*order);
        }
    }
}

int OrderBook::cancelOrder(Order *order) {
    LOCK_BOOK();
    if(order->remaining>0) {
        order->cancel();
        auto orders = order->side == BUY ? &bids : &asks;
        orders->removeOrder(order);
        listener.onOrder(*order);
        return 0;
    } else {
        return -1;
    }
}

const Book OrderBook::book() {
    LOCK_BOOK();
    Book book;
    auto snap = [](const PriceLevels& src,std::vector<BookLevel> &dst, std::vector<long> &oids) {
        for(auto level=src.levels.begin();level!=src.levels.end();level++) {
            auto orders = level->second;
            int quantity(0);
            for(auto node=orders.head;node!=nullptr;node=node->next) {
                quantity = quantity + node->order->remainingQuantity();
                oids.push_back(node->order->exchangeId);
            }
            dst.push_back(BookLevel(level->first,quantity));
        }
    };
    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());
    snap(bids,book.bids,book.bidOrderIds);
    snap(asks,book.asks,book.askOrderIds);
    return book;
}

const Order OrderBook::getOrder(Order* order) {
    LOCK_BOOK();
    return *order;
}
