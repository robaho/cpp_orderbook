#pragma once 

#include <stdexcept>
#include <cstddef>
#include <atomic>

#include "orderbook.h"

#define MAX_INSTRUMENTS 1024

/** Book is a lock-free map of instrument -> OrderBook */
class BookMap {
    std::atomic<OrderBook*> table[MAX_INSTRUMENTS];
public:
    BookMap() {
        for(int i=0;i<MAX_INSTRUMENTS;i++) table[i].store(nullptr);
    }
    OrderBook* getOrCreate(const std::string_view &instrument,OrderBookListener &listener) {
        auto hash = std::hash<std::string_view>{}(instrument);
        const auto start = hash%MAX_INSTRUMENTS;
        auto book = table[start].load();
        if(book!=nullptr && book->instrument==instrument) return book;
        auto new_book = new OrderBook(std::string(instrument),listener);
        auto index = start;
        while(true) {
            if(book!=nullptr) {
                index = (index + 1) % MAX_INSTRUMENTS;
                if(index==start) throw new std::runtime_error("no room in books map");
                book = table[index].load();
                if(book!=nullptr && book->instrument==instrument) return book;
            }
            if(book==nullptr) {
                OrderBook* tmp = nullptr;
                if(table[index].compare_exchange_strong(tmp,new_book)) return new_book;
                if(tmp->instrument==instrument) {
                    delete(new_book);
                    return tmp;
                }
            }
        }
    };
    OrderBook* get(const std::string_view &instrument) {
        auto hash = std::hash<std::string_view>{}(instrument);
        const auto start = hash%MAX_INSTRUMENTS;
        auto index = start;
        while(true) {
            auto book = table[index].load();
            if(book==nullptr) return nullptr;
            if(book->instrument==instrument) return book;
            index = (index + 1) % MAX_INSTRUMENTS;
            if(index==start) return nullptr;
        }
    }
    std::vector<std::string> instruments() {
        std::vector<std::string> result;
        for(int i=0;i<MAX_INSTRUMENTS;i++) {
            auto book = table[i].load();
            if(book!=nullptr) result.push_back(book->instrument);
        }
        return result;
    }
};