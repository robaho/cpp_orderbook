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

This later revisions of this project were born out of a job rejection. An order book is a fairly common "task" when interviewing in fintech as it is so pervasive. I was lucky enough to have feedback through a secondary source that listed items like "used string", and used dynamic memory.

If you review the performance details it's pretty clear that the optimal data structure is dependent upon the histogram of the operations performed and the size of the order book. If you're implementing an exchange, you need to account for every price level, if you're a buy side firm, normally 10 levels on either side is sufficient to implement most strategies. For example, using a vector with pointers is best when matching, but a vector with structs is best for the cancel operations. So the operation biasing requirements control which implementation is optimal.

This particular firm provided no guidelines on these parameters, and the test data was blind, and then rejected the solution as inefficient without any in-person discussion of the design, constraints, etc. Imo it's a classic blunder in hiring - when the interviewer only accepts/understands the solution they expect. In fairness, it took several revisions of the code to be able to make these claims - but isn't that what optimizing engineering is?

Anyway, I hope its helpful to others when faced with a similar problem.

See `benchmark_test.cpp` and `benchmark_multithread_test.cpp` for the performance tests.

## Next Steps

Add FIX protocol acceptor to enable testing with `go-trader` client.
