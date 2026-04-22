## Goal

Evaluate the performance impact of branch misprediction compared to branch-free arithmetic in a tight loop, using Google Benchmark.

## Setup

- Language: C++
- Compiler: clang++-17 -O3 -mavx2
- Dataset size: N = 2^22 (~4M elements)
- Hardware: 12-core CPU @ 4.8 GHz
- Benchmark framework: Google Benchmark

```bash
$ export GBENCH_DIR=/home/kowal/Desktop/Git/google-benchmark
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include branches.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o branches
$ ./branches 
2026-04-22T14:56:32+02:00
Running ./branches
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.94, 0.67, 0.34
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
------------------------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------
BM_add_multiply/4194304            3102217 ns      3101910 ns          228 items_per_second=1.35217G/s
BM_branch_not_predicted/4194304   13138481 ns     13118759 ns           57 items_per_second=319.718M/s
BM_branch_predicted/4194304        2781418 ns      2780728 ns          253 items_per_second=1.50835G/s
```

## Results

| Benchmark            | Time     | Throughput     |
| -------------------- | -------- | -------------- |
| Branch-free          | ~3.1 ms  | ~1.35B ops/sec |
| Unpredictable branch | ~13.1 ms | ~320M ops/sec  |
| Predictable branch   | ~2.7 ms  | ~1.50M ops/sec |

## Key Insights
Modern CPUs rely heavily on branch prediction to keep pipelines full. When prediction fails:
- pipeline is flushed
- speculative work is discarded

Even though the second loop does similar logical work, it is much slower due to control flow, not computation.

## Takeaway

Control flow is often more expensive than computation.
Designing code for performance is not just about fewer operations —
it's about writing code that the CPU can execute predictably and continuously.

**Instruction pipelining:** is a CPU optimization technique where multiple instruction stages (fetch, decode, execute, etc.) are processed in parallel, similar to an assembly line.
Instead of waiting for one instruction to fully complete, the CPU overlaps execution of multiple instructions, significantly increasing throughput.

**Keep the CPU busy by working on different stages of multiple instructions at the same time.**

**Out-of-order execution:** (OoO) allows the CPU to execute instructions in a different order than they appear in the program, as long as data dependencies are respected.
If one instruction is waiting (e.g., for memory), the CPU can execute other independent instructions instead of stalling.

**Maximize CPU utilization by dynamically reordering instructions to avoid idle cycles.**

**Branch prediction:** A CPU technique that guesses the outcome of a branch (e.g. if statement) before it is known, so execution can continue without waiting.

**Speculative execution:** The CPU executes instructions ahead of time based on branch prediction. If the prediction was correct → results are kept. If wrong → results are discarded.

**Pipeline flush:** When a branch prediction is wrong, the CPU must discard all speculatively executed instructions and restart from the correct path. This causes a performance penalty (wasted cycles).