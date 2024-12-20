FROM gcc:latest

RUN apt-get update && \
    apt-get install -y libboost-test-dev make

COPY . /app
WORKDIR /app

RUN make -j8 -f Makefile.gcc clean all
CMD ["./bin/benchmark_test"]