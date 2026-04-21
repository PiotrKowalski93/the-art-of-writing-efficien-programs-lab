# LLVM-MCA (Machine Code Analyzer)

## What is LLVM-MCA?

**LLVM-MCA (Machine Code Analyzer)** is a performance analysis tool from the LLVM project that evaluates how compiled machine code (assembly) is executed on a specific CPU microarchitecture.

It simulates the behavior of a CPU pipeline and provides insights into how instructions are dispatched, executed, and retired. Instead of running the code, it models execution based on known characteristics of the target processor.

---

## Why is it useful?

Modern CPUs are complex and highly optimized, using features like:

* out-of-order execution
* instruction pipelining
* multiple execution units
* speculative execution

Because of this, understanding performance by just reading source code or even assembly is not enough.

**LLVM-MCA helps answer questions like:**

* What is the throughput of this code?
* Where are the bottlenecks in the pipeline?
* Is the code limited by execution units, latency, or dispatch width?
* How efficiently is the CPU utilized?

---

## How it works

LLVM-MCA takes assembly code as input and simulates execution for a given CPU model.

Typical workflow:

```bash
$ clang -O3 -S test.cpp -o test.s
$ llvm-mca -mcpu=skylake test.s
```

It then produces a report including:

* **Instruction throughput**
* **Latency analysis**
* **uOps (micro-operations) breakdown**
* **Port pressure (execution unit usage)**
* **Pipeline bottlenecks**

---

## Key concepts

### 1. Throughput

The number of cycles required to execute a block of instructions repeatedly.

### 2. Latency

The delay between instruction input and when its result becomes available.

### 3. Micro-operations (uOps)

Complex instructions are broken down into smaller operations executed by the CPU.

### 4. Port pressure

Modern CPUs have multiple execution ports. LLVM-MCA shows how heavily each port is used, helping identify resource contention.

---

## Limitations

* It does **not execute code**, only simulates it
* It does **not account for memory hierarchy effects** (cache misses, RAM latency)
* Results depend on the accuracy of the CPU model
* Real-world performance may differ due to system-level effects

---

## Summary

LLVM-MCA is a powerful tool for **static performance analysis at the machine code level**.
It allows developers to reason about CPU execution behavior and optimize code with a deep understanding of modern processor architectures.

It is widely used in high-performance domains where **every CPU cycle matters**.

## Setup

```bash
$ apt search llvm-
$ sudo apt install llvm-19 llvm-19-tools llvm-19-examples
```
Predefined assebly comments for the llvm-mca:

```cpp
#define MCA_START __asm volatile("# LLVM-MCA-BEGIN");
#define MCA_END __asm volatile("# LLVM-MCA-END");
```

Running analyzer:
```bash
$ clang++-17 superscalar.cpp -g -O3 -mavx2 --std=c++17 -mllvm -x86-asm-syntax=intel -S -o - | llvm-mca-19 -mcpu=btver2 -timeline
```

- ```-S``:generate assembly
- ```-o -```:output to stdout
- ```| ```:pipe->pass assembler to llvm-mca-19
- ```-mcpu```:cpu model
- ```-timeline```:symulation cycle by cycle

## Notes:

Tested for 
```cpp
a1 += p1[i] + p2[i];
``` 

```bash
[1]    [2]    [3]    [4]    [5]    [6]    Instructions:
 1      3     1.00    *                   mov   rcx, qword ptr [r14 + 8*rax]
 1      4     1.00    *                   add   rcx, qword ptr [rbx + 8*rax]
 1      6     1.00    *      *            add   qword ptr [rsp], rcx

Iterations:        100
Instructions:      300
Total Cycles:      308
Total uOps:        300

Dispatch Width:    2
uOps Per Cycle:    0.97
IPC:               0.97
Block RThroughput: 3.0

Instruction Info:
[1]: #uOps
[2]: Latency
[3]: RThroughput
[4]: MayLoad
[5]: MayStore
[6]: HasSideEffects (U)

# 3 operations, mov -> add -> add 
# Code was done 100 times with 308 cycles, less than one instruction per cycle on avg
...
...
...

Average Wait times (based on the timeline view):
[0]: Executions
[1]: Average time spent waiting in a scheduler's queue
[2]: Average time spent waiting in a scheduler's queue while ready
[3]: Average time elapsed from WB until retire stage

      [0]    [1]    [2]    [3]
0.     10    7.6    7.6    1.8       mov        rcx, qword ptr [r14 + 8*rax]
1.     10    8.0    1.0    0.9       add        rcx, qword ptr [rbx + 8*rax]
2.     10    8.3    0.0    0.0       add        qword ptr [rsp], rcx
       10    8.0    2.9    0.9       <total>
```

Tested for
```cpp
a1 += p1[i] + p2[i];
a2 += p1[i] * p2[i];
```

```bash
Iterations:        100
Instructions:      600
Total Cycles:      413
Total uOps:        600

Dispatch Width:    2
uOps Per Cycle:    1.45
IPC:               1.45
Block RThroughput: 4.0


Instruction Info:
[1]: #uOps
[2]: Latency
[3]: RThroughput
[4]: MayLoad
[5]: MayStore
[6]: HasSideEffects (U)

[1]    [2]    [3]    [4]    [5]    [6]    Instructions:
 1      3     1.00    *                   mov   rcx, qword ptr [rbx + 8*rax]
 1      3     1.00    *                   mov   rdx, qword ptr [r14 + 8*rax]
 1      1     0.50                        lea   rsi, [rdx + rcx]
 1      6     1.00    *      *            add   qword ptr [rsp + 8], rsi
 1      6     4.00                        imul  rdx, rcx
 1      6     1.00    *      *            add   qword ptr [rsp], rdx

Average Wait times (based on the timeline view):
[0]: Executions
[1]: Average time spent waiting in a scheduler's queue
[2]: Average time spent waiting in a scheduler's queue while ready
[3]: Average time elapsed from WB until retire stage

      [0]    [1]    [2]    [3]
0.     10    3.8    3.8    8.0       mov        rcx, qword ptr [rbx + 8*rax]
1.     10    4.9    4.9    7.9       mov        rdx, qword ptr [r14 + 8*rax]
2.     10    6.9    0.0    7.0       lea        rsi, [rdx + rcx]
3.     10    9.4    0.9    0.9       add        qword ptr [rsp + 8], rsi
4.     10    8.5    2.6    0.9       imul       rdx, rcx
5.     10    11.5   0.0    0.0       add        qword ptr [rsp], rdx
       10    7.5    2.0    4.1       <total>
```