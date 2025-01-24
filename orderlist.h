#pragma once
#include <stdexcept>
#include "order.h"

// TODO add forward_iterator support so that friend class in not needed
class OrderList {
friend class OrderBook;
private:
    Node* head=nullptr;
    Node* tail=nullptr;
    F _price;
public:
    OrderList(F price) : _price(price){}
    const F& price() const { return _price; }
    struct Iterator 
    {
        friend class OrderList;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Order*;
        using reference         = Order*&;  // or also value_type&
        value_type operator*() const { return current->order; }
        // Prefix increment
        Iterator& operator++() { current = current->next; return *this; }  
        bool operator== (const Iterator& other) const { return current == other.current; };
        operator void *() const { return current; }     
    private:
        Iterator(Node *node) : current(node){}
        Node *current;
    };
    void pushback(Order * const order) {
        auto node = &order->node;
        node->order = order;
        if(head==nullptr) {
            head=node;
            tail=node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
    }
    void remove(Order * const order){
        if(order->node.order==nullptr) throw std::runtime_error("node is null on removal");
        auto node = &order->node;
        order->node.order = nullptr;
        if(head==node) {
            head=node->next;
        } 
        if(tail==node) {
            tail=node->prev;
        }
        if(node->prev) {
            node->prev->next = node->next;
        }
        if(node->next) {
            node->next->prev = node->prev;
        }
    }
    Order* front() const {
        return head==nullptr ? nullptr : head->order;
    }
    Iterator begin() const { return Iterator(head); }
    Iterator end()   const { return Iterator(nullptr); }
};