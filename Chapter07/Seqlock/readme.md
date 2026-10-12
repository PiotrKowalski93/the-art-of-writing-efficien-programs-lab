```bash
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.39, 0.20, 0.12
---------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                          Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------------------------------------------------
BM_Locks<MutexLock<int64_t>>/2/1/real_time                   1156242 ns        27548 ns          608 items_per_second=19.892M/s
BM_Locks<MutexLock<int64_t>>/4/1/real_time                   2040105 ns        21230 ns          349 items_per_second=21.0773M/s
BM_Locks<MutexLock<int64_t>>/8/1/real_time                   4396060 ns        23107 ns          162 items_per_second=18.8805M/s
BM_Locks<MutexLock<int64_t>>/10/1/real_time                  5549484 ns        29246 ns          127 items_per_second=18.5603M/s
---------------------------------------------------------------------------------------------------------------------------------
BM_Locks<SpinLockWrapper<int64_t>>/2/1/real_time              349179 ns        17231 ns         1994 items_per_second=65.8688M/s
BM_Locks<SpinLockWrapper<int64_t>>/4/1/real_time              579763 ns        15304 ns         1181 items_per_second=74.1682M/s
BM_Locks<SpinLockWrapper<int64_t>>/8/1/real_time             2521754 ns        19485 ns          399 items_per_second=32.9136M/s
BM_Locks<SpinLockWrapper<int64_t>>/10/1/real_time            4847497 ns        22482 ns          194 items_per_second=21.2481M/s
---------------------------------------------------------------------------------------------------------------------------------
BM_Locks<SequentialLock<int64_t>>/2/1/real_time               185846 ns        16928 ns         4307 items_per_second=123.758M/s
BM_Locks<SequentialLock<int64_t>>/4/1/real_time               234491 ns        16911 ns         2919 items_per_second=183.376M/s
BM_Locks<SequentialLock<int64_t>>/8/1/real_time               350796 ns        18294 ns         2066 items_per_second=236.604M/s
BM_Locks<SequentialLock<int64_t>>/10/1/real_time              386656 ns        18150 ns         1832 items_per_second=266.387M/s
---------------------------------------------------------------------------------------------------------------------------------
BM_Locks<SequentialLockOneWriter<int64_t>>/2/1/real_time      115669 ns        16168 ns         6093 items_per_second=198.843M/s
BM_Locks<SequentialLockOneWriter<int64_t>>/4/1/real_time      173871 ns        16309 ns         3772 items_per_second=247.31M/s
BM_Locks<SequentialLockOneWriter<int64_t>>/8/1/real_time      278868 ns        15491 ns         2469 items_per_second=297.632M/s
BM_Locks<SequentialLockOneWriter<int64_t>>/10/1/real_time     332456 ns        15786 ns         2091 items_per_second=309.815M/s
---------------------------------------------------------------------------------------------------------------------------------
```