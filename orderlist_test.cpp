#define BOOST_TEST_MODULE orderlist
#include <boost/test/included/unit_test.hpp>

#include "orderbook.h"
#include "test.h"

BOOST_AUTO_TEST_CASE( orderlist ) {
    OrderList list;
    BOOST_TEST(list.front()==nullptr);
    auto o = new TestOrder(1,100,10,BUY);
    list.pushback(o);
    auto o2 = new TestOrder(2,100,10,BUY);
    list.pushback(o2);
    BOOST_TEST(list.front()==o);
    list.remove(o);
    BOOST_TEST(list.front()==o2);
    list.remove(o2);
    BOOST_TEST(list.front()==nullptr);
}
