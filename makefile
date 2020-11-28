FLAGS = -g -Wall -Werror -std=c++14
RUNNAME = cache-sim
.PHONY: cache-sim clean run

cache-sim: Caches.o
	g++ $(FLAGS) Caches.o -o cache-sim

Caches.o: Caches.cpp
	g++ -c $(FLAGS) Caches.cpp -o Caches.o

run: cache-sim
	./$(RUNNAME)

clean:
	rm *.o $(RUNNAME)

memcheck: cache-sim
	valgrind --leak-check=yes ./$(RUNNAME)
