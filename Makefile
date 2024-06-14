CC=g++
CC_FLAGS=-std=c++20 -Wall -O3 -fopenmp -march=native
ifeq ($(DEBUG),1)
CC_FLAGS+=-g
endif

# If the first argument is "run"...
ifeq (run,$(firstword $(MAKECMDGOALS)))
# use the rest as arguments for "run"
RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
# ...and turn them into do-nothing targets
$(eval $(RUN_ARGS):;@:)
endif

sha3: sha3.cpp sha3.h utils.h # Build sha3 application
	${CC} ${CC_FLAGS} sha3.cpp -o $@

rebuild: # Force rebuild
	${CC} ${CC_FLAGS} sha3.cpp -o sha3

all: sha3 # Same as 'make sha3'

clean: # Remove sha3 binary
	rm sha3 2>/dev/null || true

run: sha3 # Run sha3 application. Rebuild if there are any changes to code
	./sha3 ${RUN_ARGS}

help: # Show help for each of the Makefile recipes.
	@grep -E '^[a-zA-Z0-9 -]+:.*#'  Makefile | sort | while read -r l; do printf "\033[1;32m$$(echo $$l | cut -f 1 -d':')\033[00m:$$(echo $$l | cut -f 2- -d'#')\n"; done

.PHONY: run rebuild help clean
