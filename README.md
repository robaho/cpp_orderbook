## Summary

This is a C++ implementation of a financial exchange. It is inspired by the Go version in [go-trader](https://github.com/robaho/go-trader).

It uses [cpp_fixed](https://github.com/robaho/cpp_fixed) to perform fixed decimal point integer math.

It supports limit and market orders.

## Building

Remove `fixed.h` , and run `make all` to obtain the latest version of the fixed point decimal library.

## Usage

See `exchange.h` for the public api. `orderbook.h` is the internal single threaded order book management.

## Performance

Running OSX on a 4 GHz Quad-Core Intel Core i7 with a single instrument.

```
Insert orders at 5.5M per second.
Insert orders with 30% trade match, 4M per second.
Cancel orders at 4M per second.
```

Running same hardware with an instrument per core:
```
Insert orders at 21M per second.
Insert orders with 31% trade match, 17M per second.
Cancel orders at 15M per second.
```
Since the test is fully cpu bound, only the 4 real cores can be utilized but still achieving a linear speedup. This is achieved using a few highly efficient lock-free structures.

It could probably bit a faster, but the design biases towards readability and safety.

See `benchmark_test.cpp` and `benchmark_multithread_test.cpp` for the performance tests.

## Next Steps

Add FIX protocol acceptor to enable testing with `go-trader` client.
