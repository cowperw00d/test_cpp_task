CC = g++
CFLAGS = -O2 -std=gnu++17

all: main

main: clean
	$(CC) $(CFLAGS) main.cpp -o main

clean:
	rm -rf main

test: main
	./main test_file.txt

.PHONY: all clean test