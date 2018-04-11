CC = clang++ -g -Wall -Wextra -std=c++14

Term.o: Term.cc
	$(CC) -c $< -o $@

Polynomial.o: Polynomial.cc
	$(CC) -c Polynomial.cc

TestRunner.o: TestRunner.cc
	$(CC) -c TestRunner.cc

test.out: Term.o Polynomial.o TestRunner.o
	$(CC) -o test.out Term.o Polynomial.o TestRunner.o

clean:
	rm -f *.o
	rm -f *.out
