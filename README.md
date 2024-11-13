# PolyLin

Lightweight and fast polynomial time solutions for monitoring linearizability of concurrent histories.

## Histories

Histories are text files that provide a **data type** as header and **operations** on a single object of the stated data type in the following rows. Yes, we assume all operations to be completed by the end of the history.

**Data types** are prefixed with `#` followed by any of the supported tags:

1. `stack` for stack
2. `queue` for queue
3. `pqueue` for priority queue
4. `set` for set

**Operations** are denoted by method, value, start time, and end time in that order. Refer to examples in `tests` directory for supported methods for a given data type.

### Example

```
# stack
push 1 1 2
peek 2 3 4
pop 2 5 6
pop 1 7 8
```

## Usage

```bash
-bash-4.2$ ./polylin [-it] <history_file>
```

### Options

- `-i`: incremental mode, report time where violation is first observed, $-1$ if no violation is observed
- `-t`: report time taken in seconds

### Output

The standard output shall be in the form:

```
"%d %d %f\n", <linearizability>, <time of violation>, <time taken>
```

_linearizability_ prints `1` when input history is linearizable, `0` otherwise.

```bash
# linearizable queue history with 10000 operations
-bash-4.2$ ./polylin -it ../tests/queue/queue_10000.1.log
1 -1 0.24
```

## Time Complexity

| Data Type | Time Complexity |
| --------- | --------------- |
| Set       | $O(n)$          |
| Stack     | $O(n\log{n})$   |
| Queue     | $O(n\log{n})$   |
| P. Queue  | $O(n\log{n})$   |

An extra $O(\log{n})$ factor (for performing binary search) is required for incremental search of violation. It reports the minimal time for which a violation is observed, taking into consideration all return values of pending operations.

## Starting Up

Clone repository locally.

```bash
-bash-4.2$ git clone https://github.com/solarrabbit99/PolyLin.git
-bash-4.2$ cd PolyLin
```

The following instructions are based on linux systems.

### Requirements

The main engine (`src`) is written in C++20. Scripts for generating testcases (`scripts`) are written in python.

- `cmake` version >= 3.16
- `python` version >= 3.8

### Building

```bash
# create build directory within project directory
-bash-4.2$ mkdir build && cd build
# build executable
-bash-4.2$ cmake .. && make
```
