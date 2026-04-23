
```bash
$ export GBENCH_DIR=/home/kowal/Desktop/Git/google-benchmark
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include branches.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o branches
$ ./branches 
2026-04-23T21:40:21+02:00
Running ./branches
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.89, 0.62, 0.49
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
------------------------------------------------------------------------------------------------------
Benchmark                                            Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------------
BM_branch_predicted/4194304                    2632317 ns      2632187 ns          263 items_per_second=1.59347G/s
BM_false_branch/4194304                       12703942 ns     12700455 ns           55 items_per_second=330.248M/s
BM_false_branch_temp_var/4194304              12721548 ns     12720324 ns           55 items_per_second=329.732M/s
BM_false_branch_conditions_in_array/4194304    2416091 ns      2415975 ns          291 items_per_second=1.73607G/s
BM_false_branch_sum/4194304                    3396447 ns      3396201 ns          209 items_per_second=1.235G/s
BM_false_branch_bitwise/4194304                3306140 ns      3306024 ns          210 items_per_second=1.26869G/s
```

| Benchmark                             | Time (ns) | Throughput |
| ------------------------------------- | --------: | ---------: |
| `BM_branch_predicted`                 |   ~2.6 ms |  ~1.59 G/s |
| `BM_false_branch`                     |  ~12.7 ms |  ~0.33 G/s |
| `BM_false_branch_temp_var`            |  ~12.7 ms |  ~0.33 G/s |
| `BM_false_branch_conditions_in_array` |   ~2.4 ms |  ~1.73 G/s |
| `BM_false_branch_sum`                 |   ~3.4 ms |  ~1.23 G/s |
| `BM_false_branch_bitwise`             |   ~3.3 ms |  ~1.27 G/s |

**Key Observations**
- Branch prediction works extremely well for predictable patterns, up to ~5× faster than unpredictable branches.
- Random / mispredicted branches are very expensive, pipeline flushes significantly degrade performance.
- Eliminating branches improves performance
- Using precomputed conditions (conditions_in_array) restores full speed
- Bitwise / branchless approaches (bitwise, sum) are much faster than naive branching
- Temporary variables do not help, BM_false_branch ≈ BM_false_branch_temp_var