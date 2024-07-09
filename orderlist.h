#pragma once
#include <stdexcept>
#include <unordered_map>

#include "order.h"


class Node {
friend class OrderList;
friend class OrderBook;
private:
    Node* prev=nullptr;
    Node* next=nullptr;
    Node(Order * const order) : order(order){}
    Order * const order;
};

// TODO add forward_iterator support so that friend class in not needed
class OrderList {
friend class OrderBook;
private:
    Node* head=nullptr;
    Node* tail=nullptr;
    std::unordered_map<Order *,Node *> allOrders;
public:
    void pushback(Order * const order) {
        auto node = new Node(order);
        if(head==nullptr) {
            head=node;
            tail=node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
        allOrders.insert({order,node});
    }
    void remove(Order * const order){
        auto itr = allOrders.find(order);
        if(itr==allOrders.end()) throw std::runtime_error("order not found");
        auto node = itr->second;
        allOrders.erase(itr);
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
        delete node;
    }
    Order* front() {
        return head==nullptr ? nullptr : head->order;
    }
};