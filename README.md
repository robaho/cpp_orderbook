## Summary

This is a C++ implementation of a financial exchange. It is inspired by the Go version in [go-trader](https://github.com/robaho/go-trader).

It uses [cpp_fixed](https://github.com/robaho/cpp_fixed) to perform fixed decimal point integer math.

It supports limit and market orders.

## Building

Remove `fixed.h` , and run `make all` to obtain the latest version of the fixed point decimal library.

## Usage

See `exchange.h` for the public api.

## Performance

Running OSX on a single 4.0 ghz Intel processor:

```
Insert orders at 5M per second.
Insert orders with 30% trade match, 4M per second.
Cancel orders at 4M per second.
```

It could probably bit a faster, but the design biases towards readability and safety.

See `benchmark_test.cpp` and `benchmark_multithread_test.cpp` for the performance tests.

## Next Steps

Add FIX protocol acceptor to enable testing with `go-trader` client.
