CC=g++
CC_FLAGS=-std=c++20 -Wall -O3 -fopenmp -march=native

# If the first argument is "run"...
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif

sha3: sha3.cpp sha3.h utils.h
	${CC} ${CC_FLAGS} sha3.cpp -o $@

rebuild:
	${CC} ${CC_FLAGS} sha3.cpp -o sha3

all: sha3

clean:
	rm sha3

.PHONY: run rebuild

run: sha3
	./sha3 ${RUN_ARGS}
