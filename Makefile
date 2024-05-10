CC=g++
CC_FLAGS=-std=c++20 -Wall -O0

sha3: sha3.cpp
	${CC} ${CC_FLAGS} $^ -o $@

all: sha3

clean:
	rm sha3

run: sha3
	./sha3
