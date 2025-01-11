## Summary

This is a C++ implementation of a financial exchange. It is inspired by the Go version in [go-trader](https://github.com/robaho/go-trader).

It uses [cpp_fixed](https://github.com/robaho/cpp_fixed) to perform fixed decimal point integer math.

It supports limit and market orders.

## Building

Remove `fixed.h` , and run `make all` to obtain the latest version of the fixed point decimal library.

You need the [Boost Unit Testing Framework](https://www.boost.org/doc/libs/1_87_0/libs/test/doc/html/index.html) installed.

It builds using `make` and by default with CLang. There is `Makefile.gcc` to use GCC instead.

## Usage

See `exchange.h` for the public api. `orderbook.h` is the internal single threaded order book management.

## Performance

<details>
    <summary> view performance details </summary>
<br>
The PriceLevels implementation can be chosen by modifying the [typedef xxxxx PriceLevels;](https://github.com/robaho/cpp_orderbook/blob/1b57f00fe031a09c28ab0df4dcacf1f6f29e48d7/pricelevels.h#L243) in `pricelevels.h` and rebuilding.

<pre>

Using dequeue:

insert orders 1000 levels, usec per order 0.171672, orders per sec 5825061
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.231113, orders per sec 4326883
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.231527, orders per sec 4319150
insert orders 10 levels, usec per order 0.125911, orders per sec 7942117
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.187032, orders per sec 5346690
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.157627, orders per sec 6344090

Using vector:

insert orders 1000 levels, usec per order 0.14516, orders per sec 6888964
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.219419, orders per sec 4557484
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.185811, orders per sec 5381812
insert orders 10 levels, usec per order 0.114813, orders per sec 8709837
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.163958, orders per sec 6099119
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.121652, orders per sec 8220169

Using vector with structs:

insert orders 1000 levels, usec per order 0.154717, orders per sec 6463397
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.381428, orders per sec 2621729
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.189634, orders per sec 5273315
insert orders 10 levels, usec per order 0.118384, orders per sec 8447073
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.151584, orders per sec 6597011
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.126755, orders per sec 7889235

Using map:

insert orders 1000 levels, usec per order 0.148093, orders per sec 6752518
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.262041, orders per sec 3816202
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.207532, orders per sec 4818534
insert orders 10 levels, usec per order 0.117244, orders per sec 8529235
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.195957, orders per sec 5103173
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.123975, orders per sec 806614

Using map with structs:

insert orders 1000 levels, usec per order 0.145034, orders per sec 6894920
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.226345, orders per sec 4418032
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.202022, orders per sec 4949955
insert orders 10 levels, usec per order 0.117999, orders per sec 8474676
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.173564, orders per sec 5761556
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.123793, orders per sec 8078001
</pre>
</details>

Running OSX on a 4 GHz Quad-Core Intel Core i7 with a single instrument.

```
Insert orders at 5.5M - 8.5M per second
Insert orders with 30% trade match, 4M - 6.5M per second
Cancel orders at 4M - 8.5M per second
```

Running same hardware with an instrument per core:
```
Insert orders at less than 50 nanoseconds per insert, more than 22M orders per second.
Insert orders with 31% trade match at less than 65 nanoseconds per insert/match.
Cancel orders at less than 70 nanoseconds per cancel.
```

The test is fully cpu bound, and achieves a near 100% speedup per core. This is made possible using a few highly efficient lock-free structures.

See `benchmark_test.cpp` and `benchmark_multithread_test.cpp` for the performance tests.

## Motivation

<details>
    <summary>Why did I do this?</summary>
<br/>
Later revisions (including many micro-optimizations) of this project were born out of a job rejection in an effort to see "just how fast I could go". An order book is a fairly common exercise when interviewing in fintech. Having written multiple production grade orderbook implementations in my career, I was baffled at the rejection.

I was lucky enough to have feedback through a secondary source that listed items like "used string", and "used dynamic memory". Where the reviewer errored is that they did not go deep enough to understand _how_ they were being used. Almost all string usages were references at near zero cost, except for the callbacks where copies were used for safety. The dynamic memory solution uses a custom arena allocator - again a near zero cost - but it makes the solution more complex.

A review of performance details above makes it clear that the optimal data structure is dependent upon the distribution of the operations performed and the typical size of the order book. If you're implementing an exchange, you need to account for every price level, if you're a buy side firm, normally 10 levels on either side is sufficient to implement most strategies. As with all engineering, the optimum solution is based on the particular usage and constraints.

The interview exercise provided no guidelines on these parameters, and the test data was blind. My solution was rejected as inefficient. As I considered the solution _fast enough_ in the general case, I decided to keep the code "clean", and I focused on other areas like test cases and documentation. For instance, the reason a cancel in the current solution is "super fast" is because the Order maintains pointers back into the OrderList for fast removal - which is somewhat brittle and not intuitively obvious - it doesn't _read well_ BUT IT'S **FAST!**

The real problem is that when you create a "production grade" solution with auditing, logging, and all sorts of IO, the cost of those elements dwarf any speed gains achieved via the micro-optimizations, and getting those right - which many developers can't do - often leads to 10x performance improvements over the "fast" solution, usually because refactoring a "simple" solution is so much easier. Furthermore, making a solution "safe" comes at a cost, optimizing engineers know how to balance these - or you can go with the "unsafe" solution but be prepared to be the next Knight Capital. 

In my opinion, it's a fairly common blunder in hiring - where the interviewer only accepts/understands the solution they expect to see - a 15 minute conversation might have resulted in a different outcome.

Anyway, I hope this is helpful to others when faced with a similar problem.

</details>

## Next Steps

Add FIX protocol acceptor to enable testing with `go-trader` client.
