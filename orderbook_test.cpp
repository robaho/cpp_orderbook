#define BOOST_TEST_MODULE orderbook
#include <boost/test/included/unit_test.hpp>

#include "orderbook.h"

class TestOrder : public Order {
public:
    TestOrder(long id,F price,int quantity,Side side) : Order("", "dummy",price,quantity,side,id) {}
};

BOOST_AUTO_TEST_CASE( orderbook_cancel ) {
    OrderBookListener listener;
    OrderBook ob(listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);

    ob.cancelOrder(o1);

    auto levels = ob.book();
    BOOST_TEST(levels.bids.size()==0);

    auto o2 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o2);
    auto o3 = new TestOrder(1,90,10,BUY);
    ob.insertOrder(o3);
    auto o4 = new TestOrder(1,80,10,BUY);
    ob.insertOrder(o4);

    ob.cancelOrder(o3);

    auto book = ob.book();

    BOOST_TEST(book.bids.size()==2);
    BOOST_TEST(book.bids[0].price==100);
    BOOST_TEST(book.bids[1].price==80);
}

BOOST_AUTO_TEST_CASE( booklevels ) {
    OrderBookListener listener;
    OrderBook ob(listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);

    auto levels = ob.book();

    BOOST_TEST(levels.bids[0].price==100);
    BOOST_TEST(levels.bids[0].quantity==10);
}

BOOST_AUTO_TEST_CASE( booklevels_sum ) {
    OrderBookListener listener;
    OrderBook ob(listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);
    auto o2 = new TestOrder(2,100,10,BUY);
    ob.insertOrder(o2);

    auto levels = ob.book();
    
    BOOST_TEST(levels.bids[0].price==100);
    BOOST_TEST(levels.bids[0].quantity==20);
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

    auto levels = ob.book();
    
    BOOST_TEST(levels.bids[0].price==200);
    BOOST_TEST(levels.bids[0].quantity==30);
    BOOST_TEST(levels.bids[1].price==100);
    BOOST_TEST(levels.bids[1].quantity==20);
}
