CXX = clang++
INCLUDES = -I ../cpp_fixed
# CXXFLAGS = -std=c++20 -Wall -O0 -fsanitize=address -fno-omit-frame-pointer -pedantic-errors -g ${INCLUDES}
# CXXFLAGS = -std=c++20 -Wall -pedantic-errors -g ${INCLUDES}
CXXFLAGS = -std=c++20 -O3 -Wall -pedantic-errors -g -fno-permissive ${INCLUDES}
# CXXFLAGS = -std=c++20 -O3 -fprofile-generate -Wall -pedantic-errors -g ${INCLUDES}
# CXXFLAGS = -std=c++20 -O3 -fprofile-use=default.profdata -Wall -pedantic-errors -g ${INCLUDES}

TEST_SRCS = ${wildcard *_test.cpp}
TEST_OBJS = $(addprefix bin/, $(TEST_SRCS:.cpp=.o))
TEST_MAINS = $(addprefix bin/, $(TEST_SRCS:.cpp=))

HEADERS = ${wildcard *.h}

SRCS = orderbook.cpp exchange.cpp

OBJS = $(addprefix bin/, $(SRCS:.cpp=.o))

LIB = bin/orderbook.a

.PRECIOUS: bin/%.o

all: $(TEST_MAINS) ${LIB}
	@echo compile finished

test: ${TEST_MAINS}

run_tests: ${TEST_MAINS}
	for main in $^ ; do \
		$$main; \
	done

${LIB}: ${OBJS}
	ar r ${LIB} ${OBJS}

bin/%_test: bin/%_test.o ${LIB}
	${CXX} ${LDFLAGS} ${CXXFLAGS} $@.o ${LIB} -o $@ 

bin/%.o: %.cpp ${HEADERS}
	@ mkdir -p bin
	${CXX} ${CXXFLAGS} -c $(notdir $(basename $@).cpp) -o $@

clean:
	rm -rf bin *~.


