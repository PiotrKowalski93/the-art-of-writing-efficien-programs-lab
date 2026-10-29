# The Art of Writing Efficient Programs – Lab

This repository contains hands-on experiments and implementations based on the book *"The Art of Writing Efficient Programs"*.
The goal is not just to read, but to **understand performance at a low level** by building, measuring, and analyzing real code.

## Objectives

* Understand how modern CPUs execute code
* Explore **cache behavior**, **branch prediction**, and **memory access patterns**
* Learn how to write **predictable and efficient code**
* Measure performance using benchmarks instead of assumptions
* Build intuition useful for **low-latency systems**

## What’s inside

* Implementations of examples from the book (C / C++)
* Micro-benchmarks for performance testing
* Experiments with:

  * Cache locality
  * False sharing
  * Branch prediction
  * Memory layout
  * Compiler optimizations
* [Concurrency experiments](#concurrency) in Chapter 07

## Tech stack

* C / C++
* CMake
* Google Benchmark
* Linux, Ubuntu

## Concurrency

Concurrency examples and benchmarks are grouped in [Chapter 07](Chapter07/readme.md):

* [SpinLock implementation and notes](Chapter07/SpinLock.md)
* [Mutex stack benchmark](Chapter07/MutexStack/mutex_stack_benchmarks.cpp)
* [Shared-mutex stack benchmark](Chapter07/SharedMutexStack/readwrite_stack_benchmarks.cpp)
* [Seqlock benchmarks](Chapter07/Seqlock/locks_benchmarks.cpp)
* [ConcurrentAppendDeque benchmark](Chapter07/ConcurrentAppendDeque/ConcurrentAppendDeque_benchmarks.cpp)
* [RingQueue benchmark](Chapter07/RingQueue/ringqueue_benchmarks.cpp)
* [Double-checked locking notes](<Chapter07/Double Checked Locking Pattern/Double-Checked Locking Pattern.md>)

## Reference

* *The Art of Writing Efficient Programs* by Fedor G. Pikus


## Setup

```bash
sudo apt install clang-17
clang++-17 --version
```

AVX2 Support check:

```bash
lscpu | grep avx2
```