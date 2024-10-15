#pragma once

#include "order.h"
#include <vector>
#include <array>

/**
 * @brief maintains map of Orders. since the exchange ID is a sequential long starting at 0 this is trivial.
 * @todo if the ID were changed to arbitrary alphanumeric, change this to an closed address hash table.
 */
class OrderMap {
    std::vector<std::array<Order*,1000>> blocks;
public:
    void add(Order *order){
        long id = order->exchangeId;
        int block = id / 1000;
        if(block>=blocks.size()) {
            blocks.push_back(std::array<Order*,1000>());
        }
        int index = id % 1000;
        blocks[block][index] = order;
    }
    Order* get(long exchangeId) {
        int block = exchangeId / 1000;
        if(block>=blocks.size()) return nullptr;
        return blocks[block][exchangeId % 1000];
    }
};