CC = g++-5 -O3 -Wall -Wextra -std=c++14

Term.o: Term.cc
	$(CC) -c $< -o $@

Polynomial.o: Polynomial.cc
	$(CC) -c Polynomial.cc

TestRunner.o: TestRunner.cc
	$(CC) -c TestRunner.cc

test.out: Term.o Polynomial.o TestRunner.o
	$(CC) -o test.out Term.o Polynomial.o TestRunner.o

sausage.out: Term.o Polynomial.o PReach.cc
	$(CC) -o sausage.out -I lemon/include -L lemon/lib Term.o Polynomial.o PReach.cc -lemon

clean:
	rm -f *.o
	rm -f *.out
