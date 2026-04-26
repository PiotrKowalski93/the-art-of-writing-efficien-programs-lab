### Setup

The benchmark compares sequential memory writes to:

- std::list<unsigned long>
- std::vector<unsigned long>

```bash
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include list_and_vector.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o listAndVector
$ ./listAndVector 
2026-04-26T10:37:34+02:00
Running ./listAndVector
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.56, 0.19, 0.12
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
---------------------------------------------------------------------------------
Benchmark                                       Time             CPU   Iterations
---------------------------------------------------------------------------------
BM_write_list<unsigned long>/1048576      6598872 ns      6598731 ns          157
BM_write_vector<unsigned long>/1048576     317733 ns       317700 ns         2215
```
## Results

| Benchmark                        | Time (ns)     | Iterations |
| -------------------------------- | ------------- | ---------- |
| `BM_write_list<unsigned long>`   | ~6,598,731 ns | 157        |
| `BM_write_vector<unsigned long>` | ~317,700 ns   | 2215       |

## Notes

**Memory Layout**
- std::vector:
    - Contiguous memory (array-like)
    - Excellent spatial locality
- std::list:
    - Each element allocated separately (heap)
    - Pointer-based structure (linked list)

## Takeaways
- Prefer std::vector for performance-critical code
- std::list is almost never a good choice in modern systems
- Cache locality is often the dominant factor in performance