**Counters that perf measue, easier to search when in file:**

```
$ perf list > perf_list.txt
```

**Using perf on app with conters:**

```
$ perf stat -e cache-misses,cpu-cycles,branch-misses ./example
```

**Running perf profiler:**

```
$ clang++-17 -v -g -O3 -mavx2 -Wall -pedantic  program.cpp -o example
$ perf record ./example
$ perf report
```
We can see assembler and % of execution time per line.