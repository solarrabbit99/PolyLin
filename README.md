# PolyLin

Polynomial time solutions to monitoring linearizability of concurrent histories.

The main engine (`src`) is written in C++20. Scripts for generating testcases (`scripts`) are written in python.

## Time Complexity

The individual algorithms are implemented with the following time complexities:

|          | Dist Vals  |
| -------- | ---------- |
| Stack    | O(n log n) |
| Queue    | O(n log n) |
| P. Queue | O(n log n) |
| Deque    | O(n^4)     |

## Starting Up

Clone repository locally.

```bash
-bash-4.2$ git clone https://github.com/solarrabbit99/PolyLin.git
-bash-4.2$ cd PolyLin
```

The following instructions are based on linux systems.

### Requirements

- `cmake` version >= 3.16
- `python` version >= 3.8

### Engine

For building engine:

```bash
# create build directory within project directory
-bash-4.2$ mkdir build && cd build
# build executable
-bash-4.2$ cmake .. && make
```

## Usage

Engine returns `1` when input history is linearizable, `0` otherwise.

```bash
# running engine on a linearizable queue history with 10000 operations
-bash-4.2$ ./engine ../tests/queue/queue_10000.1.log
1
```

Generate testcases using scripts:

```bash
-bash-4.2$ cd ../scripts
# make generator scripts executable
-bash-4.2$ chmod +x *HistGen.py
# generate linearizable history of 100000 operations with -l flag
-bash-4.2$ ./QueueHistGen.py -l ../tests/queue/queue_100000.1.log 100000
# generate non-linearizable history of 100000 operations without -l flag
-bash-4.2$ ./QueueHistGen.py ../tests/queue/queue_100000.0.log 100000
```
