#define BOOST_TEST_MODULE orderbook
#include <boost/test/included/unit_test.hpp>

#include "order.h"
#include "test.h"

BOOST_AUTO_TEST_CASE( order_basic ) {
    auto order = TestOrder("myorder",1,100,10,Order::BUY);
    BOOST_REQUIRE_EQUAL(order.orderId(),"myorder");
    auto order2 = TestOrder("myorder2",1,100,10,Order::BUY);
    BOOST_REQUIRE_EQUAL(order2.orderId(),"myorder2");
}
