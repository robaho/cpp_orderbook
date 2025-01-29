#pragma once

#include <atomic>
#include <mutex>

#if defined(__aarch64__)
#else
#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>
#endif

class SpinLock {
private:
    std::atomic_flag mutex;
public:
    void lock() {
        while(true) {
            while(mutex.test(std::memory_order_acquire)) {
                #if defined(__aarch64__)
                                asm volatile("yield" ::: "memory");
                #else
                                _mm_pause();
                #endif
            }
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