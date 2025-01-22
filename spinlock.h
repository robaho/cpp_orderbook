#pragma once

#include <atomic>
#include <chrono>
#include <emmintrin.h>
#include <thread>
#include <immintrin.h>
#include <iostream>

class SpinLock {
private:
    std::atomic_flag mutex;
public:
    void lock() {
        while(true) {
            while(mutex.test(std::memory_order_acquire)) _mm_pause();
            if(!mutex.test_and_set(std::memory_order_acquire)) {
                return;
            } else {
                mutex.wait(true);
                // std::this_thread::yield();
            }
        }
    }
    bool try_lock() {
        return mutex.test_and_set()==false;
    }
    void unlock() {
        mutex.clear(std::memory_order_release);
        mutex.notify_one();
    }
    bool is_locked() {
        return mutex.test();
    }
};

typedef std::lock_guard<SpinLock> Guard;