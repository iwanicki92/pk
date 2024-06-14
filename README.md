# SHA3 (Keccak)

## Description

This repository contains SHA3 algorithm implementation given by the professor.
I had to implement calculating digest of message and calculate original message
when given hash and length of hashed message by brute-forcing every combination.

| Message Length |                      Hash                       |
|----------------|-------------------------------------------------|
|        2       | 50 F9 10 74 B8 57 FB 7E 64 8F 7C C4 31 DC 5F 8A |
|        3       | 0E 39 A2 16 C8 34 A0 1F 6C 7D 69 90 EE AA E4 BE |
|        4       | F1 6B 29 69 FB DE B8 ED 36 D8 50 67 EC B9 B9 9F |
|        5       | F6 62 77 1A 39 4C 3E 63 A1 05 F8 87 78 BD A0 5F |
|        6       | 98 C3 CA 3D 8B 43 99 5C E1 A2 07 EF CA E5 88 1E |
|        7       | C2 70 1C 6A 6A 6C 18 0F 23 69 36 28 9C DB 6A 73 |
|        8       | 33 FE 44 57 C9 FD B4 F6 2B 80 A7 76 BC EA 4C E8 |

Implementation can calculate digest of message up to 255 characters, limited by
use of `uint8_t` type. There are 2 versions implemented with tags `v1.0-general`
and `v1.0-fixed`. Both are described below

### v1.0-general

This version uses `std::vectors` to store most of it's data, message included.
Thanks to that this version can calculate hashes or reverse hashes from user
input. To display help use `-h` or `--help` argument when running the
application.

```text
Usage:
1. ./sha3
2. ./sha3 message
3. ./sha3 message_length message_hash
4. ./sha3 (-h|--help)

1. Find messages from built-in hashes.
2. Calculate hash of <message>.
3. Find message of length <message_length> from <message_hash>.
4. Print this help.
```

### v1.0-fixed

This version doesn't accept user input and can only calculate original messages
from built-in hashes. Uses `std::array` instead of `std::vector`. This version
is somewhat faster at reversing hashes than `v1.0-general`.

```text
Usage:
1. ./sha3
4. ./sha3 (-h|--help)

1. Find messages from built-in hashes.
4. Print this help.
```

### Multithreading

Both versions are multithreaded and will use all available cores to reverse
hash. To change number of threads used set `OMP_NUM_THREADS` variable to desired
number of thread e.g.

```shell
OMP_NUM_THREADS=1 ./sha3
```

## Building

This application was tested on x86_64 `Fedora 39` and `Fedora 40`. Application was
built using `GNU Make 4.4.1` and `gcc version 14.1.1`.

To build this application just use included Makefile

```shell
$ make help
all: Same as 'make sha3'
clean: Remove sha3 binary
help: Show help for each of the Makefile recipes.
rebuild: Force rebuild
run: Run sha3 application. Rebuilt if there are any changes to code
sha3: Build sha3 application
```

## Performance

All tests were made on undervolted (-0.05 V) `Intel(R) Core(TM) i7-14700K` on
`v1.0-fixed` branch.

In the table below results will contain time it took to find message and
percentage of total number of permutations checked.
Percentage is only approximation and in the worst case could be off by number
of threads used. Times starting with star `*` are predicted times needed to
find message in worst case scenario (not very accurate). Each additional
character means that there are 79 times more possible permutations to check
(Each character can only be from set of 79 characters).

| Length | Message | v1.0-fixed 1 thread  | v1.0-general 28 threads | v1.0-fixed 28 threads |
|--------|---------|----------------------|-------------------------|-----------------------|
|   2    | v#      | 0.0014 s   (80%)     | 0.0012 s    (69%)       | 0.008 s    (2%)       |
|   3    | XXy     | 0.007 s    (7%)      | 0.008 s     (89%)       | 0.016 s   (83%)       |
|   4    | -ZH{    | 6 s        (91%)     | 0.43 s      (60%)       | 0.36 s    (71%)       |
|   5    | NTT@_   | *8.6 m               | 1.6 s        (0%)       | 1.08 s     (0%)       |
|   6    | 24MoIb  | *11.4 h              | *1.2 h                  | 15 m      (28%)       |
|   7    | ?       | *37 days             | *4 days                 | *20 h                 |
|   8    | ?       | *8 years             | *323 days               | *65 days              |
