#include "orderbook.h"

#include <sys/param.h>

#include "order.h"

#define LOCK_BOOK() std::lock_guard<std::recursive_mutex> lock(mu)

void OrderBook::insertOrder(Order* order) {
    auto list = order->side == BUY ? &bids : &asks;
    list->insertOrder(order);
    listener.onOrder(*order);
    matchOrders(order->side);
}

void OrderBook::matchOrders(Side aggressorSide) {
    while (!bids.empty() && !asks.empty()) {
        auto bid = bids.front();
        auto ask = asks.front();

        if (bid->_price >= ask->_price) {
            int qty = MIN(bid->remaining, ask->remaining);
            F price = MIN(bid->_price, ask->_price);

            Order* aggressor = aggressorSide == BUY ? bid : ask;
            Order* opposite = aggressorSide == BUY ? ask : bid;

            bid->fill(qty);
            ask->fill(qty);

            const Trade trade(price, qty, *aggressor, *opposite);

            if (bid->remaining == 0) {
                bids.removeOrder(bid);
            }
            if (ask->remaining == 0) {
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
    auto orders = aggressorSide == BUY ? &bids : &asks;
    if (!orders->empty()) {
        auto order = orders->front();
        if (order->isMarket()) {
            order->cancel();
            orders->removeOrder(order);
            listener.onOrder(*order);
        }
    }
}

void OrderBook::quote(const std::string& sessionId, F bidPrice, int bidQuantity, F askPrice, int askQuantity, const std::string& quoteId) {
    auto key = SessionQuoteId(sessionId, quoteId);
    auto itr = quotes.find(key);
    if (itr == quotes.end()) {
        QuoteOrders q;
        if (bidQuantity != 0) {
            q.bid = new (allocateOrder()) Order(sessionId, quoteId, instrument, bidPrice, bidQuantity, BUY, 0);
            q.bid->isQuote = true;
            bids.insertOrder(q.bid);
        }
        if (askQuantity != 0) {
            q.ask = new (allocateOrder()) Order(sessionId, quoteId, instrument, askPrice, askQuantity, SELL, 0);
            q.bid->isQuote = true;
            asks.insertOrder(q.ask);
        }
        quotes[key] = q;
    } else {
        auto bid = itr->second.bid;
        auto ask = itr->second.ask;
        if(bid->isOnList()) {
            bids.removeOrder(bid);
        }
        if(ask->isOnList()) {
            asks.removeOrder(ask);
        }
        if (bidQuantity != 0) {
            bid->_price = bidPrice;
            bid->_quantity = bidQuantity;
            bid->remaining = bidQuantity;
            bid->filled = 0;
            bids.insertOrder(bid);
            matchOrders(Side::BUY);
        }
        if (askQuantity != 0) {
            ask->_price = askPrice;
            ask->_quantity = askQuantity;
            ask->remaining = askQuantity;
            ask->filled = 0;
            asks.insertOrder(ask);
            matchOrders(Side::SELL);
        }
    }
}

int OrderBook::cancelOrder(Order* order) {
    if (order->remaining > 0) {
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
    auto snap = [](const PriceLevels& src, std::vector<BookLevel>& dst, std::vector<long>& oids) {
        auto fn = [&](const OrderList* orders) {
            int quantity(0);
            for (auto itr = orders->begin(); itr != orders->end(); ++itr) {
                quantity = quantity + (*itr)->remainingQuantity();
                oids.push_back((*itr)->exchangeId);
            }
            dst.push_back({orders->price(), quantity});
        };
        src.forEach(fn);
    };
    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());
    snap(bids, book.bids, book.bidOrderIds);
    snap(asks, book.asks, book.askOrderIds);
    return book;
}

const Order OrderBook::getOrder(Order* order) {
    return *order;
}
