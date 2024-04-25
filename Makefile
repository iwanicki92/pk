CC=g++
CC_FLAGS=-std=c++20

sha3: sha3.cpp
	${CC} ${CC_FLAGS} $^ -o $@

all: sha3

run: sha3
	./sha3
