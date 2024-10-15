#pragma once

#include <atomic>
#include <emmintrin.h>
#include <thread>
#include <immintrin.h>
#include <iostream>

class Guard {
friend class SpinLock;
public:
    ~Guard() {
        mutex.store(false);
        // std::cout << "unlocked by guard\n";
    }
private:
    Guard(std::atomic<bool> &mutex) : mutex(mutex) {}
    std::atomic<bool> &mutex;        
};

class SpinLock {
private:
    std::atomic<bool> mutex;
public:
    Guard lock() {
        while(true) {
            bool unlocked = false;
            if(mutex.compare_exchange_strong(unlocked,true)) {
                return Guard(mutex);
            }
            while(mutex.load()) _mm_pause();
            // while(mutex.load()) std::this_thread::yield();
        }
    }
    bool try_lock() {
        bool unlocked = false;
        return mutex.compare_exchange_strong(unlocked,true);
    }
    void unlock() {
        mutex.store(false);
    }
    bool is_locked() {
        return mutex.load();
    }
};