#pragma once

#include <__iterator/concepts.h>
#include <__iterator/iterator_traits.h>
#include <deque>
#include "order.h"
#include "orderlist.h"

struct price_compare {
    explicit price_compare(bool ascending) : ascending(ascending) {}
    template<class T, class U>
    inline bool operator()(const T& t, const U& u) const {
        return (ascending) ? t->price < u : t->price > u;
    }
    const bool ascending;
};

struct IPriceLevels {
    void insertOrder(Order *order);
    void removeOrder(Order *order);
    int size() const;
    bool empty() const;
    Order* front() const;
    // iterate the price levels in order calling the function on each
    void forEach(void fn(OrderList*)) const;
};

class DequeuePriceLevels : public IPriceLevels {
private:
    const price_compare cmpFn;
    std::deque<OrderList*> levels;
public:
    DequeuePriceLevels(bool ascending) : cmpFn(ascending) {}
    void insertOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price, cmpFn);
        OrderList *list;
        if(itr==levels.end() || (*itr)->price!=order->price) {
            list = new OrderList(order->price);
            levels.insert(itr,list);
        } else list = *itr;
        list->pushback(order);
    }
    void removeOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price, cmpFn);
        if(itr==levels.end() || (*itr)->price!=order->price) throw new std::runtime_error("price level for order does not exist");
        OrderList *list = *itr;
        list->remove(order);
        if(list->front()==nullptr) {
            levels.erase(itr);
            free(list);
        }
    }
    Order* front() const {
        auto itr = levels.begin();
        if(itr==levels.end()) return nullptr;
        return (*itr)->front();
    }
    bool empty() const {
        return levels.empty();
    }
    int size() const {
        return levels.size();
    }
    void forEach(std::function<void(const OrderList*)> fn) const {
        for(auto itr=levels.begin();itr!=levels.end();itr++) {
            fn(*itr);
        }
    }
};

typedef DequeuePriceLevels PriceLevels;