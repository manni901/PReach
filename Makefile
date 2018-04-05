CC = clang++ -g -Wall -Wextra -std=c++14

Term.o: Term.cc
	$(CC) -c $< -o $@

clean:
	rm *.o
