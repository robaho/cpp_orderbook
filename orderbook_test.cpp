#include "order.h"
#define BOOST_TEST_MODULE orderbook
#include <boost/test/included/unit_test.hpp>

#include "orderbook.h"

class TestOrder : public Order {
public:
    TestOrder(long id,F price,int quantity,Side side) : Order("", "dummy",price,quantity,side,id) {}
};

BOOST_AUTO_TEST_CASE( booklevels ) {
    OrderBookListener listener;
    OrderBook ob(listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);

    auto levels = ob.book().levels();

    BOOST_TEST(levels.bids[0].first==100);
    BOOST_TEST(levels.bids[0].second==10);
}

BOOST_AUTO_TEST_CASE( booklevels_sum ) {
    OrderBookListener listener;
    OrderBook ob(listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);
    auto o2 = new TestOrder(2,100,10,BUY);
    ob.insertOrder(o2);

    auto levels = ob.book().levels();
    
    BOOST_TEST(levels.bids[0].first==100);
    BOOST_TEST(levels.bids[0].second==20);
}

BOOST_AUTO_TEST_CASE( booklevels_multiple ) {
    OrderBookListener listener;
    OrderBook ob(listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);
    auto o2 = new TestOrder(2,100,10,BUY);
    ob.insertOrder(o2);
    auto o3 = new TestOrder(2,200,30,BUY);
    ob.insertOrder(o3);

    auto levels = ob.book().levels();
    
    BOOST_TEST(levels.bids[0].first==200);
    BOOST_TEST(levels.bids[0].second==30);
    BOOST_TEST(levels.bids[1].first==100);
    BOOST_TEST(levels.bids[1].second==20);
}



