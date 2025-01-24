#pragma once

#include <deque>
#include <map>
#include <functional>
#include "order.h"
#include "orderlist.h"

// The purpose of PriceLevels is to allow a compile time indirection to test implementation using different data structures

struct price_compare {
    explicit price_compare(bool ascending) : ascending(ascending) {}
    template<class T, class U>
    inline bool operator()(const T& t, const U& u) const {
        return (ascending) ? t->price() < u : t->price() > u;
    }
    const bool ascending;
};
struct price_compare_struct {
    explicit price_compare_struct(bool ascending) : ascending(ascending) {}
    template<class T, class U>
    inline bool operator()(const T& t, const U& u) const {
        return (ascending) ? t.price() < u : t.price() > u;
    }
    const bool ascending;
};

template <typename Container>
concept ContainerOfPtr = requires(Container c) {
    typename Container::value_type;
    requires std::same_as<typename Container::value_type, OrderList*>;
};

template <typename Container>
concept ContainerOfStruct = requires(Container c) {
    typename Container::value_type;
    requires std::same_as<typename Container::value_type, OrderList>;
};

template <typename Container>
concept MapOfStruct = requires(Container c) {
    typename Container::value_type;
    requires std::same_as<typename Container::value_type, std::pair<F,OrderList>>;
};

template <typename Container>
concept MapOfPtr = requires(Container c) {
    typename Container::value_type;
    requires std::same_as<typename Container::value_type, std::pair<F,OrderList*>>;
};

template <typename ContainerOfPtr>
class PointerPriceLevels {
private:
    const price_compare cmpFn;
    ContainerOfPtr levels;
public:
    PointerPriceLevels(bool ascending) : cmpFn(ascending) {}
    void insertOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price(), cmpFn);
        OrderList *list;
        if(itr==levels.end() || (*itr)->price()!=order->price()) {
            list = new OrderList(order->price());
            levels.insert(itr,list);
        } else list = *itr;
        list->pushback(order);
    }
    void removeOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price(), cmpFn);
        if(itr==levels.end() || (*itr)->price()!=order->price()) throw new std::runtime_error("price level for order does not exist");
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

template <typename ContainerOfStruct>
class StructPriceLevels {
private:
    const price_compare_struct cmpFn;
    ContainerOfStruct levels;
public:
    StructPriceLevels(bool ascending) : cmpFn(ascending) {}
    void insertOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price(), cmpFn);
        if(itr==levels.end() || itr->price()!=order->price()) {
            OrderList list(order->price());
            list.pushback(order);
            levels.insert(itr,std::move(list));
        } else {
            itr->pushback(order);
        }
    }
    void removeOrder(Order *order) {
        auto itr = std::lower_bound(levels.begin(),levels.end(),order->price(), cmpFn);
        if(itr==levels.end() || itr->price()!=order->price()) throw new std::runtime_error("price level for order does not exist");
        itr->remove(order);
        if(itr->front()==nullptr) {
            levels.erase(itr);
        }
    }
    Order* front() const {
        auto itr = levels.begin();
        if(itr==levels.end()) return nullptr;
        return itr->front();
    }
    bool empty() const {
        return levels.empty();
    }
    int size() const {
        return levels.size();
    }
    void forEach(std::function<void(const OrderList*)> fn) const {
        for(auto itr=levels.begin();itr!=levels.end();itr++) {
            fn(&(*itr));
        }
    }
};

struct fixed_compare {
    explicit fixed_compare(bool ascending) : ascending(ascending) {}
    bool operator()(const F& t, const F& u) const {
        return (ascending) ? t < u : t > u;
    }
    const bool ascending;
};

template <typename MapOfStruct>
class MapPriceLevels {
private:
    const fixed_compare cmpFn;
    MapOfStruct levels;
public:
    MapPriceLevels(bool ascending) : cmpFn(ascending), levels(cmpFn) {}
    void insertOrder(Order *order) {
        auto itr = levels.lower_bound(order->price());
        if(itr==levels.end() || itr->first!=order->price()) {
            OrderList list(order->price());
            list.pushback(order);
            levels.insert({list.price(),std::move(list)});
        } else {
            itr->second.pushback(order);
        }
    }
    void removeOrder(Order *order) {
        auto itr = levels.lower_bound(order->price());
        if(itr==levels.end() || itr->first!=order->price()) throw new std::runtime_error("price level for order does not exist");
        itr->second.remove(order);
        if(itr->second.front()==nullptr) {
            levels.erase(itr);
        }
    }
    Order* front() const {
        auto itr = levels.begin();
        if(itr==levels.end()) return nullptr;
        return itr->second.front();
    }
    bool empty() const {
        return levels.empty();
    }
    int size() const {
        return levels.size();
    }
    void forEach(std::function<void(const OrderList*)> fn) const {
        for(auto itr=levels.begin();itr!=levels.end();itr++) {
            fn(&(itr->second));
        }
    }
};

template <typename MapOfPtr>
class MapPtrPriceLevels {
private:
    const fixed_compare cmpFn;
    MapOfPtr levels;
public:
    MapPtrPriceLevels(bool ascending) : cmpFn(ascending), levels(cmpFn) {}
    void insertOrder(Order *order) {
        auto itr = levels.lower_bound(order->price());
        if(itr==levels.end() || itr->first!=order->price()) {
            auto list = new OrderList(order->price());
            list->pushback(order);
            levels.insert({list->price(),std::move(list)});
        } else {
            itr->second->pushback(order);
        }
    }
    void removeOrder(Order *order) {
        auto itr = levels.lower_bound(order->price());
        if(itr==levels.end() || itr->first!=order->price()) throw new std::runtime_error("price level for order does not exist");
        itr->second->remove(order);
        if(itr->second->front()==nullptr) {
            levels.erase(itr);
            free(itr->second);
        }
    }
    Order* front() const {
        auto itr = levels.begin();
        if(itr==levels.end()) return nullptr;
        return itr->second->front();
    }
    bool empty() const {
        return levels.empty();
    }
    int size() const {
        return levels.size();
    }
    void forEach(std::function<void(const OrderList*)> fn) const {
        for(auto itr=levels.begin();itr!=levels.end();itr++) {
            fn(itr->second);
        }
    }
};


typedef PointerPriceLevels<std::deque<OrderList*>> DequeuePtrPriceLevels;
typedef PointerPriceLevels<std::vector<OrderList*>> VectorPtrPriceLevels;
typedef StructPriceLevels<std::vector<OrderList>> VectorPriceLevels;
typedef MapPriceLevels<std::map<F,OrderList,fixed_compare>> StdMapPriceLevels;
typedef MapPtrPriceLevels<std::map<F,OrderList*,fixed_compare>> StdMapPtrPriceLevels;

// define the PriceLevels implementation to use
typedef VectorPriceLevels PriceLevels;