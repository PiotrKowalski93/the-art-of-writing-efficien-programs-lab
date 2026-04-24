
### Basic

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


## Branchless computing

```bash
$ ./branches 
2026-04-24T07:48:58+02:00
Running ./branches
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 1.33, 0.64, 0.29
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-------------------------------------------------------------------------------------------------------
Benchmark                                             Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------
BM_branched/4194304                            12883022 ns     12881707 ns           56 items_per_second=325.602M/s
BM_branchless/4194304                           3784753 ns      3784584 ns          182 items_per_second=1.10826G/s
BM_branchless_question_mark_operator/4194304    3705658 ns      3704646 ns          189 items_per_second=1.13217G/s
```

Some complers treat ?: operator as array with index (same as in BM_branchless).
Branchless computing gives us a big performance boost, because we dont have to flush pipeline when our speculative execution is wrong.

```bash
$ ./branches 
2026-04-24T08:18:44+02:00
Running ./branches
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.44, 0.20, 0.25
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
------------------------------------------------------------------------------------------------
Benchmark                                      Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------
BM_branched_add_or_multiply/4194304     13601090 ns     13599795 ns           50 items_per_second=308.409M/s
BM_branchless_add_or_multiply/4194304    5431270 ns      5431054 ns          134 items_per_second=772.282M/s
```

```cpp
// BM_branched_add_or_multiply
for (size_t i = 0; i < N; ++i) {
    if (b1[i]) {
        a1 += p1[i] - p2[i];
    } else {
        a2 += p1[i] * p2[i];
    }
}

// BM_branchless_add_or_multiply
unsigned long* a[2] = {&a1, &a2};
for (size_t i = 0; i < N; ++i) {
    unsigned long s[2] = {p1[i] - p2[i], p1[i] * p2[i]};
    a[b1[i]] += s[b1[i]];
}
```