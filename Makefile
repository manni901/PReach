CC = g++ -O3 -Wall -Wextra -std=c++14

LEMON_INCLUDE = -I lemon/include -L lemon/lib

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = googletest

# Where to find user code.
USER_DIR = .

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = TermTest

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h


# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CC) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CC) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

TermTest: Term.o TermTest.cc gtest_main.a
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

Term.o: Term.cc
	$(CC) -c $< -o $@

Polynomial.o: Polynomial.cc
	$(CC) -c Polynomial.cc

SausageSolver.o: SausageSolver.cc
	$(CC) $(LEMON_INCLUDE) -c SausageSolver.cc

SamplingSolver.o: SamplingSolver.cc
	$(CC) $(LEMON_INCLUDE) -c SamplingSolver.cc

TestRunner.o: TestRunner.cc
	$(CC) -c TestRunner.cc

Graph.o: Graph.cc
	$(CC) $(LEMON_INCLUDE) -c Graph.cc -lemon

main: Term.o Polynomial.o Graph.o SausageSolver.o SamplingSolver.o PReach.cc
	$(CC) -o $@ $(LEMON_INCLUDE) $^ -lemon

clean:
	rm -f *.o
	rm -f *.out
	rm -f $(TESTS) gtest_main.a
