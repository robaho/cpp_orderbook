#include "order.h"
#include "orderbook.h"
#define BOOST_TEST_MODULE ordermap
#include <boost/test/included/unit_test.hpp>

#include "test.h"

BOOST_AUTO_TEST_CASE( ordermap_basic ) {
    OrderMap map;

    auto o = new TestOrder(1,100,10,BUY);
    BOOST_TEST(map.get(1)==nullptr);
    map.add(o);
    BOOST_TEST(map.get(1)==o);
}

