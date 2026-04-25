# Memory Access Benchmarks

## Overview

This benchmark compares sequential and random memory access patterns
for reads and writes across different data sizes and types (`int` vs `long`).

The goal is to understand how memory access patterns impact performance.

## Key Findings

### 1. Sequential Access is Bandwidth-Bound

Sequential reads achieve stable high throughput:

- ~34 GiB/s for `int`
- ~65–70 GiB/s for `long`

Performance remains nearly constant regardless of data size.

This is due to:
- Hardware prefetching
- Efficient cache line utilization
- High spatial locality

### 2. Random Access is Latency-Bound

Random reads degrade dramatically with data size:

- From ~17 GiB/s (small data)
- Down to <1 GiB/s (large data)

This is caused by:
- Cache misses (L1 → L2 → L3 → RAM)
- Lack of prefetching
- Poor spatial locality

### 3. Cache Hierarchy Becomes Visible

Random access clearly exposes cache boundaries:

- L1 (~32KB): fast
- L2 (~256KB): slowdown begins
- L3 (~12MB): further degradation
- RAM: severe performance drop

### 4. Writes Are Slower Than Reads

Sequential writes are ~2x slower than reads due to:

- Write-allocate policy 
- Additional memory traffic

### 5. Random Writes Are the Worst Case

Random writes show the lowest performance:

- Down to ~300–500 MiB/s

This combines:
- High latency
- No locality
- Write overhead

### 6. Data Type Matters Less Than Expected

- `long` achieves higher bandwidth (more bytes per access)
- But items/sec is similar to `int`

This shows that:
> CPU operates on cache lines, not individual elements

## Key Takeaway

> Performance is dominated by memory access patterns.

- Sequential access → high throughput (bandwidth-bound)
- Random access → high latency (latency-bound)

## Notes

**Write-allocate policy:** A write-allocate policy, often referred to as "fetch-on-write," is a cache management strategy used when a processor performs a write operation that misses the cache (the data is not currently in the cache). In this policy, the cache controller fetches the entire cache block containing the target address from the main memory and loads it into the cache before updating it.

**Cache prefetching** is a technique used by central processing units (CPUs) to boost execution performance by fetching instructions or data from their primary or main storage in slower memory to a faster local memory before it is actually needed.

## Benchmark results

```bash
$ ./sequential 
2026-04-25T17:39:21+02:00
Running ./sequential
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.77, 0.58, 0.28
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
--------------------------------------------------------------------------------------------------------
Benchmark                                              Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------------------------
BM_sequential_read<unsigned int>/1024               27.4 ns         27.4 ns     25557757 bytes_per_second=34.8143Gi/s items_per_second=9.34539G/s
BM_sequential_read<unsigned int>/2048               56.7 ns         56.7 ns     12703900 bytes_per_second=33.6582Gi/s items_per_second=9.03506G/s
BM_sequential_read<unsigned int>/4096                111 ns          111 ns      6355564 bytes_per_second=34.3774Gi/s items_per_second=9.2281G/s
BM_sequential_read<unsigned int>/8192                221 ns          221 ns      3158355 bytes_per_second=34.5775Gi/s items_per_second=9.28182G/s
BM_sequential_read<unsigned int>/16384               446 ns          446 ns      1584922 bytes_per_second=34.2054Gi/s items_per_second=9.18193G/s
BM_sequential_read<unsigned int>/32768               895 ns          895 ns       792402 bytes_per_second=34.0852Gi/s items_per_second=9.14967G/s
BM_sequential_read<unsigned int>/65536              1768 ns         1768 ns       398604 bytes_per_second=34.5305Gi/s items_per_second=9.26922G/s
BM_sequential_read<unsigned int>/131072             3531 ns         3531 ns       199371 bytes_per_second=34.5715Gi/s items_per_second=9.28022G/s
BM_sequential_read<unsigned int>/262144             7035 ns         7036 ns        99002 bytes_per_second=34.701Gi/s items_per_second=9.31499G/s
BM_sequential_read<unsigned int>/524288            14379 ns        14377 ns        48610 bytes_per_second=33.9622Gi/s items_per_second=9.11665G/s
BM_sequential_read<unsigned int>/1048576           28635 ns        28638 ns        24485 bytes_per_second=34.1007Gi/s items_per_second=9.15385G/s
BM_sequential_read<unsigned int>/2097152           57104 ns        57094 ns        12250 bytes_per_second=34.2088Gi/s items_per_second=9.18285G/s
BM_sequential_read<unsigned int>/4194304          114310 ns       114323 ns         6096 bytes_per_second=34.1685Gi/s items_per_second=9.17202G/s
BM_sequential_read<unsigned int>/8388608          239728 ns       239741 ns         2950 bytes_per_second=32.5873Gi/s items_per_second=8.74759G/s
BM_sequential_read<unsigned int>/16777216         480341 ns       480384 ns         1412 bytes_per_second=32.5261Gi/s items_per_second=8.73115G/s
BM_sequential_read<unsigned int>/33554432         949605 ns       949406 ns          714 bytes_per_second=32.9153Gi/s items_per_second=8.83564G/s
BM_sequential_read<unsigned int>/67108864        1961571 ns      1961780 ns          358 bytes_per_second=31.8588Gi/s items_per_second=8.55204G/s
BM_sequential_read<unsigned int>/134217728       3901102 ns      3901041 ns          137 bytes_per_second=32.0427Gi/s items_per_second=8.6014G/s
BM_sequential_read<unsigned int>/268435456       7998905 ns      7997378 ns           68 bytes_per_second=31.2602Gi/s items_per_second=8.39136G/s
BM_sequential_read<unsigned int>/536870912      17032678 ns     17031030 ns           34 bytes_per_second=29.3582Gi/s items_per_second=7.88078G/s
BM_sequential_read<unsigned int>/1073741824     38494890 ns     38497128 ns           14 bytes_per_second=25.976Gi/s items_per_second=6.97287G/s
BM_sequential_read<unsigned long>/1024              13.8 ns         13.8 ns     48969341 bytes_per_second=68.9356Gi/s items_per_second=9.25238G/s
BM_sequential_read<unsigned long>/2048              27.6 ns         27.6 ns     24784429 bytes_per_second=69.1786Gi/s items_per_second=9.285G/s
BM_sequential_read<unsigned long>/4096              54.7 ns         54.7 ns     12709936 bytes_per_second=69.7093Gi/s items_per_second=9.35622G/s
BM_sequential_read<unsigned long>/8192               110 ns          110 ns      6371429 bytes_per_second=69.6096Gi/s items_per_second=9.34285G/s
BM_sequential_read<unsigned long>/16384              221 ns          221 ns      3202224 bytes_per_second=69.1699Gi/s items_per_second=9.28382G/s
BM_sequential_read<unsigned long>/32768              442 ns          442 ns      1581933 bytes_per_second=69.0324Gi/s items_per_second=9.26537G/s
BM_sequential_read<unsigned long>/65536              883 ns          882 ns       798874 bytes_per_second=69.1625Gi/s items_per_second=9.28284G/s
BM_sequential_read<unsigned long>/131072            1767 ns         1767 ns       397592 bytes_per_second=69.0745Gi/s items_per_second=9.27102G/s
BM_sequential_read<unsigned long>/262144            3556 ns         3556 ns       198888 bytes_per_second=68.6628Gi/s items_per_second=9.21576G/s
BM_sequential_read<unsigned long>/524288            7354 ns         7354 ns        96150 bytes_per_second=66.3924Gi/s items_per_second=8.91103G/s
BM_sequential_read<unsigned long>/1048576          14630 ns        14628 ns        47591 bytes_per_second=66.759Gi/s items_per_second=8.96025G/s
BM_sequential_read<unsigned long>/2097152          29295 ns        29298 ns        23924 bytes_per_second=66.6635Gi/s items_per_second=8.94743G/s
BM_sequential_read<unsigned long>/4194304          57929 ns        57930 ns        12031 bytes_per_second=67.4307Gi/s items_per_second=9.05039G/s
BM_sequential_read<unsigned long>/8388608         124418 ns       124409 ns         5644 bytes_per_second=62.797Gi/s items_per_second=8.42848G/s
BM_sequential_read<unsigned long>/16777216        248443 ns       248448 ns         2822 bytes_per_second=62.8905Gi/s items_per_second=8.44101G/s
BM_sequential_read<unsigned long>/33554432        499556 ns       499495 ns         1333 bytes_per_second=62.5632Gi/s items_per_second=8.39709G/s
BM_sequential_read<unsigned long>/67108864        998411 ns       998519 ns          662 bytes_per_second=62.5927Gi/s items_per_second=8.40105G/s
BM_sequential_read<unsigned long>/134217728      2015248 ns      2015019 ns          332 bytes_per_second=62.0342Gi/s items_per_second=8.32608G/s
BM_sequential_read<unsigned long>/268435456      4153748 ns      4153948 ns          166 bytes_per_second=60.1837Gi/s items_per_second=8.07772G/s
BM_sequential_read<unsigned long>/536870912      8861453 ns      8859928 ns           57 bytes_per_second=56.4339Gi/s items_per_second=7.57443G/s
BM_sequential_read<unsigned long>/1073741824    18977308 ns     18976844 ns           34 bytes_per_second=52.6958Gi/s items_per_second=7.07271G/s
BM_sequential_write<unsigned int>/1024              54.6 ns         54.6 ns     12747264 bytes_per_second=17.4615Gi/s items_per_second=4.68729G/s
BM_sequential_write<unsigned int>/2048               110 ns          110 ns      6373071 bytes_per_second=17.3142Gi/s items_per_second=4.64773G/s
BM_sequential_write<unsigned int>/4096               219 ns          219 ns      3184194 bytes_per_second=17.4124Gi/s items_per_second=4.67411G/s
BM_sequential_write<unsigned int>/8192               438 ns          438 ns      1601329 bytes_per_second=17.4065Gi/s items_per_second=4.67253G/s
BM_sequential_write<unsigned int>/16384              878 ns          878 ns       800995 bytes_per_second=17.3754Gi/s items_per_second=4.66416G/s
BM_sequential_write<unsigned int>/32768             1757 ns         1757 ns       397883 bytes_per_second=17.3646Gi/s items_per_second=4.66126G/s
BM_sequential_write<unsigned int>/65536             3736 ns         3736 ns       187777 bytes_per_second=16.3391Gi/s items_per_second=4.38599G/s
BM_sequential_write<unsigned int>/131072            7504 ns         7505 ns        93407 bytes_per_second=16.2656Gi/s items_per_second=4.36627G/s
BM_sequential_write<unsigned int>/262144           15519 ns        15521 ns        45275 bytes_per_second=15.73Gi/s items_per_second=4.22249G/s
BM_sequential_write<unsigned int>/524288           32449 ns        32452 ns        21547 bytes_per_second=15.0461Gi/s items_per_second=4.03891G/s
BM_sequential_write<unsigned int>/1048576          65367 ns        65371 ns        10695 bytes_per_second=14.9388Gi/s items_per_second=4.01011G/s
BM_sequential_write<unsigned int>/2097152         130990 ns       130976 ns         5325 bytes_per_second=14.9121Gi/s items_per_second=4.00294G/s
BM_sequential_write<unsigned int>/4194304         261468 ns       261490 ns         2664 bytes_per_second=14.9384Gi/s items_per_second=4.01G/s
BM_sequential_write<unsigned int>/8388608         566451 ns       566382 ns         1249 bytes_per_second=13.7937Gi/s items_per_second=3.70272G/s
BM_sequential_write<unsigned int>/16777216       1510021 ns      1510112 ns          456 bytes_per_second=10.3469Gi/s items_per_second=2.77748G/s
BM_sequential_write<unsigned int>/33554432       3309520 ns      3309098 ns          211 bytes_per_second=9.44366Gi/s items_per_second=2.53501G/s
BM_sequential_write<unsigned int>/67108864       6794937 ns      6795646 ns           90 bytes_per_second=9.19707Gi/s items_per_second=2.46882G/s
BM_sequential_write<unsigned int>/134217728     13936556 ns     13934751 ns           45 bytes_per_second=8.97038Gi/s items_per_second=2.40797G/s
BM_sequential_write<unsigned int>/268435456     28945803 ns     28948736 ns           22 bytes_per_second=8.63596Gi/s items_per_second=2.3182G/s
BM_sequential_write<unsigned int>/536870912     63223529 ns     63221311 ns           10 bytes_per_second=7.90873Gi/s items_per_second=2.12298G/s
BM_sequential_write<unsigned int>/1073741824   149741693 ns    149749240 ns            4 bytes_per_second=6.67783Gi/s items_per_second=1.79257G/s
BM_sequential_write<unsigned long>/1024             27.5 ns         27.4 ns     25490553 bytes_per_second=34.7475Gi/s items_per_second=4.66373G/s
BM_sequential_write<unsigned long>/2048             54.8 ns         54.8 ns     12709165 bytes_per_second=34.778Gi/s items_per_second=4.66783G/s
BM_sequential_write<unsigned long>/4096              109 ns          109 ns      6385554 bytes_per_second=34.8777Gi/s items_per_second=4.6812G/s
BM_sequential_write<unsigned long>/8192              219 ns          219 ns      3205635 bytes_per_second=34.8142Gi/s items_per_second=4.67268G/s
BM_sequential_write<unsigned long>/16384             441 ns          441 ns      1599665 bytes_per_second=34.635Gi/s items_per_second=4.64863G/s
BM_sequential_write<unsigned long>/32768             877 ns          878 ns       789600 bytes_per_second=34.7764Gi/s items_per_second=4.66761G/s
BM_sequential_write<unsigned long>/65536            1966 ns         1966 ns       353492 bytes_per_second=31.0397Gi/s items_per_second=4.16608G/s
BM_sequential_write<unsigned long>/131072           4014 ns         4014 ns       174669 bytes_per_second=30.4095Gi/s items_per_second=4.08149G/s
BM_sequential_write<unsigned long>/262144           8519 ns         8519 ns        82215 bytes_per_second=28.6593Gi/s items_per_second=3.84659G/s
BM_sequential_write<unsigned long>/524288          18509 ns        18506 ns        37700 bytes_per_second=26.3855Gi/s items_per_second=3.5414G/s
BM_sequential_write<unsigned long>/1048576         37254 ns        37253 ns        18683 bytes_per_second=26.2142Gi/s items_per_second=3.51841G/s
BM_sequential_write<unsigned long>/2097152         74611 ns        74596 ns         9237 bytes_per_second=26.1828Gi/s items_per_second=3.5142G/s
BM_sequential_write<unsigned long>/4194304        150411 ns       150418 ns         4715 bytes_per_second=25.9692Gi/s items_per_second=3.48553G/s
BM_sequential_write<unsigned long>/8388608        347618 ns       347570 ns         2002 bytes_per_second=22.4775Gi/s items_per_second=3.01688G/s
BM_sequential_write<unsigned long>/16777216      1130793 ns      1130805 ns          615 bytes_per_second=13.8176Gi/s items_per_second=1.85456G/s
BM_sequential_write<unsigned long>/33554432      2637603 ns      2637181 ns          263 bytes_per_second=11.8498Gi/s items_per_second=1.59045G/s
BM_sequential_write<unsigned long>/67108864      5589891 ns      5590593 ns          106 bytes_per_second=11.1795Gi/s items_per_second=1.50049G/s
BM_sequential_write<unsigned long>/134217728    11611638 ns     11612203 ns           53 bytes_per_second=10.7645Gi/s items_per_second=1.44479G/s
BM_sequential_write<unsigned long>/268435456    24214567 ns     24216530 ns           27 bytes_per_second=10.3235Gi/s items_per_second=1.3856G/s
BM_sequential_write<unsigned long>/536870912    51963874 ns     51960025 ns           12 bytes_per_second=9.62278Gi/s items_per_second=1.29155G/s
BM_sequential_write<unsigned long>/1073741824  122231270 ns    122239226 ns            5 bytes_per_second=8.18068Gi/s items_per_second=1.09799G/s
```

```bash
$ ./random 
2026-04-25T17:42:55+02:00
Running ./random
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.75, 0.77, 0.43
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
----------------------------------------------------------------------------------------------------
Benchmark                                          Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------------------
BM_random_read<unsigned int>/1024               55.0 ns         55.0 ns     12383535 bytes_per_second=17.3304Gi/s items_per_second=4.65209G/s
BM_random_read<unsigned int>/2048                110 ns          110 ns      6346168 bytes_per_second=17.34Gi/s items_per_second=4.65468G/s
BM_random_read<unsigned int>/4096                220 ns          220 ns      3184728 bytes_per_second=17.3659Gi/s items_per_second=4.66162G/s
BM_random_read<unsigned int>/8192                438 ns          438 ns      1596352 bytes_per_second=17.408Gi/s items_per_second=4.67291G/s
BM_random_read<unsigned int>/16384               911 ns          911 ns       771203 bytes_per_second=16.7419Gi/s items_per_second=4.49411G/s
BM_random_read<unsigned int>/32768              1882 ns         1882 ns       371381 bytes_per_second=16.2163Gi/s items_per_second=4.35304G/s
BM_random_read<unsigned int>/65536              5249 ns         5250 ns       131838 bytes_per_second=11.626Gi/s items_per_second=3.12084G/s
BM_random_read<unsigned int>/131072            13434 ns        13432 ns        51882 bytes_per_second=9.08772Gi/s items_per_second=2.43947G/s
BM_random_read<unsigned int>/262144            23043 ns        23046 ns        30124 bytes_per_second=10.5938Gi/s items_per_second=2.84375G/s
BM_random_read<unsigned int>/524288            52932 ns        52936 ns        13288 bytes_per_second=9.22403Gi/s items_per_second=2.47606G/s
BM_random_read<unsigned int>/1048576          113044 ns       113035 ns         6218 bytes_per_second=8.63949Gi/s items_per_second=2.31915G/s
BM_random_read<unsigned int>/2097152          231196 ns       231203 ns         3016 bytes_per_second=8.44765Gi/s items_per_second=2.26765G/s
BM_random_read<unsigned int>/4194304          469241 ns       469287 ns         1489 bytes_per_second=8.3238Gi/s items_per_second=2.2344G/s
BM_random_read<unsigned int>/8388608         1597850 ns      1597948 ns          439 bytes_per_second=4.88908Gi/s items_per_second=1.3124G/s
BM_random_read<unsigned int>/16777216        8144594 ns      8145503 ns           85 bytes_per_second=1.91824Gi/s items_per_second=514.923M/s
BM_random_read<unsigned int>/33554432       22559642 ns     22556398 ns           31 bytes_per_second=1.38542Gi/s items_per_second=371.895M/s
BM_random_read<unsigned int>/67108864       53549982 ns     53553808 ns           12 bytes_per_second=1.16705Gi/s items_per_second=313.278M/s
BM_random_read<unsigned int>/134217728     156049806 ns    156064890 ns            4 bytes_per_second=820.172Mi/s items_per_second=215.003M/s
BM_random_read<unsigned int>/268435456     440671597 ns    440712255 ns            2 bytes_per_second=580.878Mi/s items_per_second=152.274M/s
BM_random_read<unsigned int>/536870912    1053020626 ns   1053097394 ns            1 bytes_per_second=486.185Mi/s items_per_second=127.45M/s
BM_random_read<unsigned int>/1073741824   2261023259 ns   2261038752 ns            1 bytes_per_second=452.889Mi/s items_per_second=118.722M/s
BM_random_read<unsigned long>/1024              27.3 ns         27.3 ns     25577296 bytes_per_second=34.8854Gi/s items_per_second=4.68224G/s
BM_random_read<unsigned long>/2048              54.8 ns         54.8 ns     12796552 bytes_per_second=34.8343Gi/s items_per_second=4.67537G/s
BM_random_read<unsigned long>/4096               109 ns          109 ns      6390997 bytes_per_second=34.8527Gi/s items_per_second=4.67785G/s
BM_random_read<unsigned long>/8192               219 ns          219 ns      3206550 bytes_per_second=34.8137Gi/s items_per_second=4.67262G/s
BM_random_read<unsigned long>/16384              447 ns          446 ns      1571897 bytes_per_second=34.1756Gi/s items_per_second=4.58698G/s
BM_random_read<unsigned long>/32768              934 ns          934 ns       748304 bytes_per_second=32.6598Gi/s items_per_second=4.38352G/s
BM_random_read<unsigned long>/65536             2645 ns         2644 ns       265202 bytes_per_second=23.0819Gi/s items_per_second=3.098G/s
BM_random_read<unsigned long>/131072            6644 ns         6645 ns       104323 bytes_per_second=18.3707Gi/s items_per_second=2.46567G/s
BM_random_read<unsigned long>/262144           14681 ns        14682 ns        47797 bytes_per_second=16.6285Gi/s items_per_second=2.23185G/s
BM_random_read<unsigned long>/524288           36638 ns        36642 ns        19068 bytes_per_second=13.3256Gi/s items_per_second=1.78854G/s
BM_random_read<unsigned long>/1048576          84015 ns        84018 ns         8247 bytes_per_second=11.6232Gi/s items_per_second=1.56004G/s
BM_random_read<unsigned long>/2097152         180057 ns       180041 ns         3871 bytes_per_second=10.8482Gi/s items_per_second=1.45602G/s
BM_random_read<unsigned long>/4194304         375943 ns       375974 ns         1865 bytes_per_second=10.3897Gi/s items_per_second=1.39448G/s
BM_random_read<unsigned long>/8388608        1193320 ns      1193190 ns          585 bytes_per_second=6.54758Gi/s items_per_second=878.801M/s
BM_random_read<unsigned long>/16777216       5549446 ns      5549366 ns          126 bytes_per_second=2.81564Gi/s items_per_second=377.908M/s
BM_random_read<unsigned long>/33554432      11435077 ns     11433753 ns           60 bytes_per_second=2.73314Gi/s items_per_second=366.835M/s
BM_random_read<unsigned long>/67108864      30552747 ns     30549819 ns           22 bytes_per_second=2.04584Gi/s items_per_second=274.588M/s
BM_random_read<unsigned long>/134217728     81779160 ns     81782608 ns            8 bytes_per_second=1.52844Gi/s items_per_second=205.144M/s
BM_random_read<unsigned long>/268435456    220519541 ns    220540114 ns            3 bytes_per_second=1.13358Gi/s items_per_second=152.147M/s
BM_random_read<unsigned long>/536870912    552447812 ns    552489896 ns            1 bytes_per_second=926.714Mi/s items_per_second=121.466M/s
BM_random_read<unsigned long>/1073741824  1183328571 ns   1183260387 ns            1 bytes_per_second=865.405Mi/s items_per_second=113.43M/s
BM_random_write<unsigned int>/1024              56.7 ns         56.7 ns     12261795 bytes_per_second=16.8137Gi/s items_per_second=4.51338G/s
BM_random_write<unsigned int>/2048               114 ns          114 ns      6124850 bytes_per_second=16.8023Gi/s items_per_second=4.51035G/s
BM_random_write<unsigned int>/4096               226 ns          226 ns      3088636 bytes_per_second=16.8636Gi/s items_per_second=4.52678G/s
BM_random_write<unsigned int>/8192               452 ns          452 ns      1549058 bytes_per_second=16.8802Gi/s items_per_second=4.53124G/s
BM_random_write<unsigned int>/16384              948 ns          948 ns       731955 bytes_per_second=16.0994Gi/s items_per_second=4.32166G/s
BM_random_write<unsigned int>/32768             4036 ns         4036 ns       174272 bytes_per_second=7.56087Gi/s items_per_second=2.0296G/s
BM_random_write<unsigned int>/65536            12669 ns        12668 ns        55094 bytes_per_second=4.81812Gi/s items_per_second=1.29335G/s
BM_random_write<unsigned int>/131072           32909 ns        32910 ns        21286 bytes_per_second=3.70924Gi/s items_per_second=995.692M/s
BM_random_write<unsigned int>/262144           85347 ns        85340 ns         8181 bytes_per_second=2.8608Gi/s items_per_second=767.941M/s
BM_random_write<unsigned int>/524288          217728 ns       217754 ns         3209 bytes_per_second=2.24235Gi/s items_per_second=601.927M/s
BM_random_write<unsigned int>/1048576         474671 ns       474640 ns         1476 bytes_per_second=2.05748Gi/s items_per_second=552.301M/s
BM_random_write<unsigned int>/2097152         972305 ns       972352 ns          711 bytes_per_second=2.00866Gi/s items_per_second=539.196M/s
BM_random_write<unsigned int>/4194304        2065797 ns      2065679 ns          341 bytes_per_second=1.89102Gi/s items_per_second=507.618M/s
BM_random_write<unsigned int>/8388608        5945537 ns      5945716 ns          117 bytes_per_second=1.31397Gi/s items_per_second=352.716M/s
BM_random_write<unsigned int>/16777216      21384986 ns     21381015 ns           33 bytes_per_second=748.327Mi/s items_per_second=196.17M/s
BM_random_write<unsigned int>/33554432      63840409 ns     63839475 ns           10 bytes_per_second=501.257Mi/s items_per_second=131.402M/s
BM_random_write<unsigned int>/67108864     154671354 ns    154679270 ns            4 bytes_per_second=413.759Mi/s items_per_second=108.465M/s
BM_random_write<unsigned int>/134217728    367731515 ns    367667259 ns            2 bytes_per_second=348.141Mi/s items_per_second=91.263M/s
BM_random_write<unsigned int>/268435456    846933323 ns    846953092 ns            1 bytes_per_second=302.26Mi/s items_per_second=79.2356M/s
BM_random_write<unsigned int>/536870912   1765168381 ns   1765136988 ns            1 bytes_per_second=290.062Mi/s items_per_second=76.0381M/s
BM_random_write<unsigned int>/1073741824  3598063115 ns   3597941049 ns            1 bytes_per_second=284.607Mi/s items_per_second=74.6081M/s
BM_random_write<unsigned long>/1024             27.3 ns         27.3 ns     25618003 bytes_per_second=34.9279Gi/s items_per_second=4.68795G/s
BM_random_write<unsigned long>/2048             58.6 ns         58.6 ns     12084638 bytes_per_second=32.5435Gi/s items_per_second=4.36791G/s
BM_random_write<unsigned long>/4096              112 ns          112 ns      6224105 bytes_per_second=34.0376Gi/s items_per_second=4.56845G/s
BM_random_write<unsigned long>/8192              227 ns          227 ns      3077338 bytes_per_second=33.5886Gi/s items_per_second=4.50819G/s
BM_random_write<unsigned long>/16384             461 ns          461 ns      1513772 bytes_per_second=33.0896Gi/s items_per_second=4.44121G/s
BM_random_write<unsigned long>/32768            2075 ns         2074 ns       337781 bytes_per_second=14.712Gi/s items_per_second=1.97461G/s
BM_random_write<unsigned long>/65536            6494 ns         6494 ns       108089 bytes_per_second=9.39904Gi/s items_per_second=1.26152G/s
BM_random_write<unsigned long>/131072          16884 ns        16884 ns        41386 bytes_per_second=7.23004Gi/s items_per_second=970.4M/s
BM_random_write<unsigned long>/262144          41704 ns        41707 ns        16793 bytes_per_second=5.8537Gi/s items_per_second=785.671M/s
BM_random_write<unsigned long>/524288         110350 ns       110357 ns         6347 bytes_per_second=4.42456Gi/s items_per_second=593.855M/s
BM_random_write<unsigned long>/1048576        241360 ns       241378 ns         2897 bytes_per_second=4.04579Gi/s items_per_second=543.017M/s
BM_random_write<unsigned long>/2097152        491185 ns       491118 ns         1420 bytes_per_second=3.97689Gi/s items_per_second=533.77M/s
BM_random_write<unsigned long>/4194304       1014530 ns      1014626 ns          688 bytes_per_second=3.84994Gi/s items_per_second=516.73M/s
BM_random_write<unsigned long>/8388608       2983484 ns      2983016 ns          235 bytes_per_second=2.61899Gi/s items_per_second=351.515M/s
BM_random_write<unsigned long>/16777216     10808232 ns     10808803 ns           64 bytes_per_second=1.44558Gi/s items_per_second=194.023M/s
BM_random_write<unsigned long>/33554432     32353030 ns     32347742 ns           21 bytes_per_second=989.25Mi/s items_per_second=129.663M/s
BM_random_write<unsigned long>/67108864     77977730 ns     77981285 ns            7 bytes_per_second=820.71Mi/s items_per_second=107.572M/s
BM_random_write<unsigned long>/134217728   185044692 ns    185066371 ns            3 bytes_per_second=691.644Mi/s items_per_second=90.6551M/s
BM_random_write<unsigned long>/268435456   418697799 ns    418649977 ns            2 bytes_per_second=611.489Mi/s items_per_second=80.1491M/s
BM_random_write<unsigned long>/536870912   890557537 ns    890610049 ns            1 bytes_per_second=574.887Mi/s items_per_second=75.3516M/s
BM_random_write<unsigned long>/1073741824 1824047125 ns   1823995004 ns            1 bytes_per_second=561.405Mi/s items_per_second=73.5845M/s
```