#include "orderbook.h"
#define BOOST_TEST_MODULE spinlock
#include <boost/test/included/unit_test.hpp>

#include "test.h"

BOOST_AUTO_TEST_CASE( spinlock_basic ) {
    SpinLock lock;

    {
        Guard guard(lock);
        BOOST_TEST(lock.try_lock()==false);
        BOOST_TEST(lock.is_locked()==true);
    }
    BOOST_TEST(lock.is_locked()==false);
    BOOST_TEST(lock.try_lock()==true);
    lock.unlock();
    BOOST_TEST(lock.try_lock()==true);
}

BOOST_AUTO_TEST_CASE( spinlock_multithread) {
    SpinLock lock;

    std::vector<std::thread> threads;

    long count = 0;

    {
        Guard guard(lock);

        auto fn = [&](){
            for(int i=0;i<1000000;i++) {
                {
                    Guard guard(lock);
                    count++;
                }
            }
        };

        threads.push_back(std::thread(fn));
        threads.push_back(std::thread(fn));
    }

    for(auto thread = threads.begin();thread!=threads.end();thread++) {
        thread->join();
    }

    BOOST_TEST(count==2000000);
}


