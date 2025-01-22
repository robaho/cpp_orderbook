#pragma once

#include "order.h"
#include <thread>
#include <vector>
#include <array>

/**
 * @brief lock-free map of external order ID -> Order
 */
class OrderMap {
    static const int TABLE_SIZE = 1000000;
    std::atomic<Order*> table[TABLE_SIZE];
public:
    void add(Order *order){
        int bucket = order->exchangeId % TABLE_SIZE;
        Order* expected = table[bucket].load();
        order->next = expected;
        while(!table[bucket].compare_exchange_weak(expected,order)) {
            order->next = expected;
        }
    }
    Order* get(long exchangeId) {
        int bucket = exchangeId % TABLE_SIZE;
        Order *order = table[bucket].load();
        while(order!=nullptr) {
            if(order->exchangeId==exchangeId) return order;
            order = order->next;
        }
        return nullptr;
    }
};