#define BOOST_TEST_MODULE exchange
#include <boost/test/included/unit_test.hpp>

#include "test.h"

struct TestListener : ExchangeListener {
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
    TestExchange ob;

    auto o1_id = ob.buy(1.0,10,"1");
    auto o2_id = ob.buy(2.0,10,"2");

    BOOST_TEST( ob.bidCount()==2);
    BOOST_TEST( ob.askCount()==0);

    BOOST_TEST( ob.bidIndex(o2_id) == 0, o2_id);
    BOOST_TEST( ob.bidIndex(o1_id) == 1, o1_id);
}

BOOST_AUTO_TEST_CASE( insertOrder_buy2 ) {
    TestExchange ob;

    auto o2_id = ob.buy(2.0,10,"2");
    auto o1_id = ob.buy(1.0,10,"1");

    // should end up with same ordering

    BOOST_TEST( ob.bidCount()==2);
    BOOST_TEST( ob.askCount()==0);

    BOOST_TEST( ob.bidIndex(o2_id) == 0);
    BOOST_TEST( ob.bidIndex(o1_id) == 1);
}
BOOST_AUTO_TEST_CASE( insertOrder_sell ) {
    TestExchange ob;

    auto o1_id = ob.sell(1.0,10,"1");
    auto o2_id = ob.sell(2.0,10,"2");

    BOOST_TEST( ob.bidCount()==0);
    BOOST_TEST( ob.askCount()==2);

    BOOST_TEST( ob.askIndex(o2_id) == 1);
    BOOST_TEST( ob.askIndex(o1_id) == 0);
}

BOOST_AUTO_TEST_CASE( insertOrder_sell2 ) {
    TestExchange ob;

    auto o2_id = ob.sell(2.0,10,"2");
    auto o1_id = ob.sell(1.0,10,"1");

    BOOST_TEST( ob.bidCount()==0);
    BOOST_TEST( ob.askCount()==2);

    BOOST_TEST( ob.askIndex(o2_id) == 1);
    BOOST_TEST( ob.askIndex(o1_id) == 0);
}

BOOST_AUTO_TEST_CASE( insertOrder_buy_same_price ) {
    TestExchange ob;

    auto o1_id = ob.buy(1.0,10,"1");
    auto o2_id = ob.buy(2.0,10,"2");
    auto o3_id = ob.buy(2.0,25,"3");

    BOOST_TEST( ob.bidCount()==2);
    BOOST_TEST( ob.book().bidOrderIds.size()==3);
    BOOST_TEST( ob.askCount()==0);

    BOOST_TEST( ob.bidIndex(o2_id) == 0);
    BOOST_TEST( ob.bidIndex(o3_id) == 1);
    BOOST_TEST( ob.bidIndex(o1_id) == 2);
}

BOOST_AUTO_TEST_CASE( fill ) {

    TestListener listener;

    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,10,"1");

    BOOST_TEST( listener.trades.size()==0);

    auto o2_id = ob.sell(0.75,10,"2");

    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( listener.trades[0].aggressor.exchangeId == o2_id);
    BOOST_TEST( listener.trades[0].opposite.exchangeId == o1_id);
}

BOOST_AUTO_TEST_CASE( partial_fill ) {

    TestListener listener;
    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,20,"1");

    BOOST_TEST( listener.trades.size()==0);

    auto o2_id = ob.sell(.75,10,"2");

    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( listener.trades[0].aggressor.exchangeId == o2_id);
    BOOST_TEST( listener.trades[0].opposite.exchangeId == o1_id);

    BOOST_TEST( ob.getOrder(o2_id).remainingQuantity() == 0);
    BOOST_TEST( ob.getOrder(o1_id).remainingQuantity() == 10);

    BOOST_TEST( ob.bidCount() == 1);
    BOOST_TEST( ob.askCount() == 0);

}

BOOST_AUTO_TEST_CASE( cancel ) {

    TestListener listener;

    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,20,"1");
    BOOST_TEST(ob.cancel(o1_id)==0);
    BOOST_TEST(ob.cancel(o1_id)==-1);

    // should be an event for the order and the cancel
    BOOST_TEST( listener.orders.size()==2);
    BOOST_TEST( ob.bidCount() == 0);
}

BOOST_AUTO_TEST_CASE( market_buy ) {

    TestListener listener;

    TestExchange ob(listener);

    ob.sell(1.0,20,"1");
    ob.marketBuy(10,"2");

    BOOST_TEST( listener.orders.size()==4);
    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( ob.bidCount() == 0);
    BOOST_TEST( ob.askCount() == 1);
}

BOOST_AUTO_TEST_CASE( market_buy_cancel_remaining ) {

    TestListener listener;

    TestExchange ob(listener);

    ob.sell(1.0,20,"1");
    ob.marketBuy(30,"2");

    BOOST_TEST( listener.orders.size()==5);
    BOOST_TEST( listener.trades.size()==1);
    BOOST_TEST( ob.bidCount() == 0);
    BOOST_TEST( ob.askCount() == 0);
}

BOOST_AUTO_TEST_CASE( market_buy_multi_level ) {

    TestListener listener;

    TestExchange ob(listener);

    ob.sell(1.0,20,"1");
    ob.sell(2.0,20,"2");

    ob.marketBuy(30,"3");

    // initial orders (3) + 2x2 updates due to trades (1 full and 1 partial) = 7 statuses
    BOOST_TEST( listener.orders.size()==7);
    BOOST_TEST( listener.trades.size()==2);
    BOOST_TEST( ob.bidCount() == 0);
    BOOST_TEST( ob.askCount() == 1);
    BOOST_TEST( ob.book().asks[0].quantity == 10);
}


BOOST_AUTO_TEST_CASE( market_buy_one_sided ) {

    TestListener listener;

    TestExchange ob(listener);

    ob.marketBuy(30,"1");

    BOOST_TEST( listener.orders.size()==2);
    BOOST_TEST( listener.trades.size()==0);
    BOOST_TEST( ob.bidCount() == 0);
    BOOST_TEST( ob.askCount() == 0);
}

BOOST_AUTO_TEST_CASE( order_immutability ) {

    TestListener listener;

    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,30,"1");
    Order order = ob.getOrder(o1_id);
    ob.sell(1.0,10,"2");
    Order latest = ob.getOrder(o1_id);

    BOOST_TEST( order.remainingQuantity()==30);
    BOOST_TEST( latest.remainingQuantity()==20);
}







