#define BOOST_TEST_MODULE orderbook
#include <boost/test/included/unit_test.hpp>

#include "orderbook.h"
#include "test.h"

BOOST_AUTO_TEST_CASE( orderbook_cancel ) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument,listener);

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
    OrderBook ob(dummy_instrument,listener);

    auto o1 = new TestOrder(1,100,10,BUY);
    ob.insertOrder(o1);

    auto levels = ob.book();

    BOOST_TEST(levels.bids[0].price==100);
    BOOST_TEST(levels.bids[0].quantity==10);
}

BOOST_AUTO_TEST_CASE( booklevels_sum ) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument,listener);

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
    OrderBook ob(dummy_instrument,listener);

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

BOOST_AUTO_TEST_CASE( booklevels_order ) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument,listener);

    ob.insertOrder(new TestOrder(1,100,10,BUY));
    ob.insertOrder(new TestOrder(1,101,10,BUY));
    ob.insertOrder(new TestOrder(1,99,10,BUY));
    ob.insertOrder(new TestOrder(1,98,10,BUY));

    ob.insertOrder(new TestOrder(1,200,10,SELL));
    ob.insertOrder(new TestOrder(1,199,10,SELL));
    ob.insertOrder(new TestOrder(1,201,10,SELL));
    ob.insertOrder(new TestOrder(1,202,10,SELL));


    auto levels = ob.book();
    
    BOOST_TEST(levels.bids[0].price==101);
    BOOST_TEST(levels.bids[1].price==100);
    BOOST_TEST(levels.bids[2].price==99);
    BOOST_TEST(levels.bids[3].price==98);

    BOOST_TEST(levels.asks[0].price==199);
    BOOST_TEST(levels.asks[1].price==200);
    BOOST_TEST(levels.asks[2].price==201);
    BOOST_TEST(levels.asks[3].price==202);
}
