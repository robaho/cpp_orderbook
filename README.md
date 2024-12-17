## Summary

This is a C++ implementation of a financial exchange. It is inspired by the Go version in [go-trader](https://github.com/robaho/go-trader).

It uses [cpp_fixed](https://github.com/robaho/cpp_fixed) to perform fixed decimal point integer math.

It supports limit and market orders.

## Building

Remove `fixed.h` , and run `make all` to obtain the latest version of the fixed point decimal library.

## Usage

See `exchange.h` for the public api. `orderbook.h` is the internal single threaded order book management.

## Performance

<details>
    <summary> view performance details </summary>
<pre>

These are available under different branches in the repo.

Using dequeue (main branch):

insert orders 1000 levels, usec per order 0.213368, orders per sec 4686734
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.289039, orders per sec 3459745
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.243564, orders per sec 4105697
insert orders 10 levels, usec per order 0.175881, orders per sec 5685652
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.248569, orders per sec 4023026
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.159411, orders per sec 6273092

Using vector:

insert orders 1000 levels, usec per order 0.194676, orders per sec 5136753
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.281497, orders per sec 3552436
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.209419, orders per sec 4775115
insert orders 10 levels, usec per order 0.151772, orders per sec 6588830
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.199023, orders per sec 5024542
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.128149, orders per sec 7803416

Using vector with structs:

insert orders 1000 levels, usec per order 0.180841, orders per sec 5529731
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.403036, orders per sec 2481170
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.178959, orders per sec 5587872
insert orders 10 levels, usec per order 0.142271, orders per sec 7028819
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.170982, orders per sec 5848572
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.116662, orders per sec 8571771

Using map:

insert orders 1000 levels, usec per order 0.194018, orders per sec 5154174
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.356886, orders per sec 2802014
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.231138, orders per sec 4326419
insert orders 10 levels, usec per order 0.159661, orders per sec 6263258
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.26134, orders per sec 3826435
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.122617, orders per sec 8155475

Using map with structs:

insert orders 1000 levels, usec per order 0.205884, orders per sec 4857101
insert orders 1000 levels with trade match % 0
insert orders 1000 levels, usec per order 0.315915, orders per sec 3165408
insert orders 1000 levels with trade match % 31
cancel orders 1000 levels, usec per order 0.253659, orders per sec 3942300
insert orders 10 levels, usec per order 0.16376, orders per sec 6106486
insert orders 10 levels with trade match % 0
insert orders 10 levels, usec per order 0.212822, orders per sec 4698753
insert orders 10 levels with trade match % 33
cancel orders 10 levels, usec per order 0.118317, orders per sec 845187
</pre>
</details>

Running OSX on a 4 GHz Quad-Core Intel Core i7 with a single instrument.

```
Insert orders at 5.5M - 7M per second
Insert orders with 30% trade match, 4M - 6M per second
Cancel orders at 4M - 8.5M per second
```

Running same hardware with an instrument per core:
```
Insert orders at less than 40 nanoseconds per insert.
Insert orders with 31% trade match at less than 50 nanoseconds per insert/match.
Cancel orders at less than 30 nanoseconds per cancel.
```

The test is fully cpu bound, and achieves a near 100% speedup per core. This is made possible using a few highly efficient lock-free structures.

This later revisions of this project were born out of a job rejection. An order book is a fairly common "task" when interviewing in fintech as it is so pervasive. I was lucky enough to have feedback through a secondary source that listed items like "used string", and used dynamic memory.

If you review the performance details it's pretty clear that the optimal data structure is dependent upon the histogram of the operations performed and the size of the order book. If you're implementing an exchange, you need to account for every price level, if you're a buy side firm, normally 10 levels on either side is sufficient to implement most strategies.

This particular firm provided no guidelines on these parameters, and the test data was blind, and then rejected the solution as inefficient without any in-person discussion of the design, constraints, etc. Imo it's a classic blunder in hiring - when the interviewer only accepts/understands the solution they expect. In fairness, it took several revisions of the code to be able to make these claims - but isn't that was optimizing engineering is?

Anyway, I hope its helpful to others when faced with a similar problem.

See `benchmark_test.cpp` and `benchmark_multithread_test.cpp` for the performance tests.

## Next Steps

Add FIX protocol acceptor to enable testing with `go-trader` client.
