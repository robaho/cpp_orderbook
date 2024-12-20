#include "orderbook.h"
#include "order.h"
#include <algorithm>
#include <mutex>
#include <sys/param.h>

#define LOCK_BOOK() std::lock_guard<std::recursive_mutex> lock(mu)

void OrderBook::insertOrder(Order *order) {
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

            const Trade trade(price,qty,*aggressor,*opposite);

            if(bid->remaining==0) {
                bids.removeOrder(bid);
            }
            if(ask->remaining==0) {
                asks.removeOrder(ask);
            }
            listener.onOrder(*bid);
            listener.onOrder(*ask);
            listener.onTrade(trade);
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
    Book book;
    auto snap = [](const PriceLevels& src,std::vector<BookLevel> &dst, std::vector<long> &oids) {
        auto fn = [&](const OrderList *orders) {
            int quantity(0);
            for(auto itr=orders->begin();itr!=orders->end();++itr) {
                quantity = quantity + (*itr)->remainingQuantity();
                oids.push_back((*itr)->exchangeId);
            }
            dst.push_back(BookLevel(orders->price(),quantity));
        };
        src.forEach(fn);
    };
    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());
    snap(bids,book.bids,book.bidOrderIds);
    snap(asks,book.asks,book.askOrderIds);
    return book;
}

const Order OrderBook::getOrder(Order* order) {
    return *order;
}
