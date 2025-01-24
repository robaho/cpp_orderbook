#include "bookmap.h"
#define BOOST_TEST_MODULE bookmap
#include <boost/test/included/unit_test.hpp>

static OrderBookListener listener;

BOOST_AUTO_TEST_CASE( books_basic ) {
    BookMap books;

    auto book = books.get("dummy");

    BOOST_TEST( book==nullptr);

    book = books.getOrCreate("dummy", listener);

    BOOST_TEST( book!=nullptr);

    auto book2 = books.getOrCreate("dummy", listener);

    BOOST_TEST( book == book2 );

    auto book3 = books.get("dummy");

    BOOST_TEST(book==book2);
    BOOST_TEST(book2==book3);
}

BOOST_AUTO_TEST_CASE( instruments ) {
    BookMap books;

    auto book = books.getOrCreate("dummy", listener);
    BOOST_TEST(book!=nullptr);
    BOOST_REQUIRE_EQUAL(books.instruments()[0],"dummy");
    BOOST_REQUIRE_EQUAL(books.instruments().size(),1);
}
