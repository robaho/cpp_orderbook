#define BOOST_TEST_MODULE orderbook
#include <boost/test/included/unit_test.hpp>

#include "orderbook.h"

struct TestListener : Listener {
    std::vector<Trade> trades;
    void onTrade(const Trade& trade) override {
        trades.push_back(trade);
    }
    std::vector<Order> orders;
    void onOrder(const Order& order) override {
        orders.push_back(order);
    }
};

BOOST_AUTO_TEST_CASE( insertOrder_buy ) {
    OrderBook ob;

    auto o1_id = ob.buy(1.0,10);
    auto o2_id = ob.buy(2.0,10);

    BOOST_TEST( ob.bidList().size()==2);
    BOOST_TEST( ob.askList().size()==0);

    BOOST_TEST( ob.bidList().indexOf(o2_id) == 0);
    BOOST_TEST( ob.bidList().indexOf(o1_id) == 1);
}

BOOST_AUTO_TEST_CASE( insertOrder_buy2 ) {
    OrderBook ob;

    auto o2_id = ob.buy(2.0,10);
    auto o1_id = ob.buy(1.0,10);

    // should end up with same ordering

    BOOST_TEST( ob.bidList().size()==2);
    BOOST_TEST( ob.askList().size()==0);

    BOOST_TEST( ob.bidList().indexOf(o2_id) == 0);
    BOOST_TEST( ob.bidList().indexOf(o1_id) == 1);
}
BOOST_AUTO_TEST_CASE( insertOrder_sell ) {
    OrderBook ob;

    auto o1_id = ob.sell(1.0,10);
    auto o2_id = ob.sell(2.0,10);

    BOOST_TEST( ob.bidList().size()==0);
    BOOST_TEST( ob.askList().size()==2);

    BOOST_TEST( ob.askList().indexOf(o2_id) == 1);
    BOOST_TEST( ob.askList().indexOf(o1_id) == 0);
}

BOOST_AUTO_TEST_CASE( insertOrder_sell2 ) {
    OrderBook ob;

    auto o2_id = ob.sell(2.0,10);
    auto o1_id = ob.sell(1.0,10);

    BOOST_TEST( ob.bidList().size()==0);
    BOOST_TEST( ob.askList().size()==2);

    BOOST_TEST( ob.askList().indexOf(o2_id) == 1);
    BOOST_TEST( ob.askList().indexOf(o1_id) == 0);
}

BOOST_AUTO_TEST_CASE( insertOrder_buy_same_price ) {
    OrderBook ob;

    auto o1_id = ob.buy(1.0,10);
    auto o2_id = ob.buy(2.0,10);
    auto o3_id = ob.buy(2.0,25);

    BOOST_TEST( ob.bidList().size()==3);
    BOOST_TEST( ob.askList().size()==0);

    BOOST_TEST( ob.bidList().indexOf(o2_id) == 0);
    BOOST_TEST( ob.bidList().indexOf(o3_id) == 1);
    BOOST_TEST( ob.bidList().indexOf(o1_id) == 2);
}

BOOST_AUTO_TEST_CASE( fill ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.buy(1.0,10);

    BOOST_TEST( listener.trades.size()==0);

    auto o2_id = ob.sell(0.75,10);

    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( listener.trades[0].aggressor == o2_id);
    BOOST_TEST( listener.trades[0].opposite == o1_id);
}

BOOST_AUTO_TEST_CASE( partial_fill ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.buy(1.0,20);

    BOOST_TEST( listener.trades.size()==0);

    auto o2_id = ob.sell(.75,10);

    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( listener.trades[0].aggressor == o2_id);
    BOOST_TEST( listener.trades[0].opposite == o1_id);

    BOOST_TEST( ob.getOrder(o2_id).remainingQuantity() == 0);
    BOOST_TEST( ob.getOrder(o1_id).remainingQuantity() == 10);

    BOOST_TEST( ob.bidList().size() == 1);
    BOOST_TEST( ob.askList().size() == 0);

}

BOOST_AUTO_TEST_CASE( cancel ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.buy(1.0,20);
    BOOST_TEST(ob.cancel(o1_id)==0);
    BOOST_TEST(ob.cancel(o1_id)==-1);

    // should be an event for the order and the cancel
    BOOST_TEST( listener.orders.size()==2);
    BOOST_TEST( ob.bidList().size() == 0);
}

BOOST_AUTO_TEST_CASE( market_buy ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.sell(1.0,20);
    auto o2_id = ob.marketBuy(10);

    BOOST_TEST( listener.orders.size()==4);
    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( ob.bidList().size() == 0);
    BOOST_TEST( ob.askList().size() == 1);
}

BOOST_AUTO_TEST_CASE( market_buy_cancel_remaining ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.sell(1.0,20);
    auto o2_id = ob.marketBuy(30);

    BOOST_TEST( listener.orders.size()==5);
    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( ob.bidList().size() == 0);
    BOOST_TEST( ob.askList().size() == 0);
}

BOOST_AUTO_TEST_CASE( market_buy_multi_level ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.sell(1.0,20);
    auto o2_id = ob.sell(2.0,20);

    auto o3_id = ob.marketBuy(30);

    // initial orders (3) + 2x2 updates due to trades (1 full and 1 partial) = 7 statuses
    BOOST_TEST( listener.orders.size()==7);
    BOOST_TEST( listener.trades.size()==2);
    BOOST_TEST( ob.bidList().size() == 0);
    BOOST_TEST( ob.askList().size() == 1);
    BOOST_TEST( ob.askList().frontOrder()->remainingQuantity() == 10);
}


BOOST_AUTO_TEST_CASE( market_buy_one_sided ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.marketBuy(30);

    BOOST_TEST( listener.orders.size()==2);
    BOOST_TEST( listener.trades.size()==0);
    BOOST_TEST( ob.bidList().size() == 0);
    BOOST_TEST( ob.askList().size() == 0);
}

BOOST_AUTO_TEST_CASE( order_immutability ) {

    TestListener listener;

    OrderBook ob(listener);

    auto o1_id = ob.buy(1.0,30);
    Order order = ob.getOrder(o1_id);
    ob.sell(1.0,10);
    Order latest = ob.getOrder(o1_id);

    BOOST_TEST( order.remainingQuantity()==30);
    BOOST_TEST( latest.remainingQuantity()==20);
}







