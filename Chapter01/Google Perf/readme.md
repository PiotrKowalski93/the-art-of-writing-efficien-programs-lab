### Build

```
clang++-17 -v -g -O3 -mavx2 -Wall -pedantic  program.cpp -o example01
```

- ```-g``` - debug symbols
- ```-O3``` - optymalisations
- ```-mavx2``` - use AVX2 (SIMD 🔥)
- ```-Wall``` - Turn on most compiler warnings
- ```-pedantic``` - enforce C++ Standard
- ```-o example``` - output

### First Profiling

```
clang++-17 -v -g -O3 -mavx2 -Wall -pedantic  program.cpp -lprofiler -o example01
```

- ```-lprofiler``` - Enables CPU Profiling. It hooks the necessary functions into executable to allow for performance profiling.

```
CPUPROFILE=prof.data ./example01
```

Read data:

```
$ CPUPROFILE=prof.data ./example01
L:1048576
N:65536
Sort time: 321ms (1262728 comparisons)
PROFILE: interrupts/evictions/bytes = 32/2/2528

$ google-pprof --text ./example01 prof.data
Using local file ./example01.
Using local file prof.data.
Total: 32 samples
      15  46.9%  46.9%       25  78.1% std::__unguarded_partition (inline)
      10  31.2%  78.1%       10  31.2% compare (inline)
       6  18.8%  96.9%        6  18.8% std::__unguarded_insertion_sort (inline)
       1   3.1% 100.0%        1   3.1% std::__move_median_to_first (inline)
       0   0.0% 100.0%       10  31.2% __gnu_cxx::__ops::_Iter_comp_iter::operator (inline)
       0   0.0% 100.0%       32 100.0% __libc_start_call_main
       0   0.0% 100.0%       32 100.0% __libc_start_main_impl
       0   0.0% 100.0%       32 100.0% _start
       0   0.0% 100.0%       32 100.0% main
       0   0.0% 100.0%       10  31.2% main::$_0::operator (inline)
       0   0.0% 100.0%        6  18.8% std::__final_insertion_sort (inline)
       0   0.0% 100.0%       26  81.2% std::__introsort_loop
       0   0.0% 100.0%       32 100.0% std::__sort (inline)
       0   0.0% 100.0%       26  81.2% std::__unguarded_partition_pivot (inline)
       0   0.0% 100.0%       32 100.0% std::sort (inline)
```

After removing one comparison in compare's loop:

```
$ CPUPROFILE=prof.data ./example01
L:1048576
N:65536
Sort time: 451ms (1262728 comparisons)
PROFILE: interrupts/evictions/bytes = 45/0/3168

$ google-pprof --text ./example01 prof.data
Using local file ./example01.
Using local file prof.data.
Total: 45 samples
      36  80.0%  80.0%       36  80.0% std::__unguarded_partition (inline)
       8  17.8%  97.8%        8  17.8% std::__unguarded_insertion_sort (inline)
       1   2.2% 100.0%        1   2.2% compare (inline)
       0   0.0% 100.0%        1   2.2% __gnu_cxx::__ops::_Iter_comp_iter::operator (inline)
       0   0.0% 100.0%       45 100.0% __libc_start_call_main
       0   0.0% 100.0%       45 100.0% __libc_start_main_impl
       0   0.0% 100.0%       45 100.0% _start
       0   0.0% 100.0%       45 100.0% main
       0   0.0% 100.0%        1   2.2% main::$_0::operator (inline)
       0   0.0% 100.0%        8  17.8% std::__final_insertion_sort (inline)
       0   0.0% 100.0%       37  82.2% std::__introsort_loop
       0   0.0% 100.0%        1   2.2% std::__move_median_to_first (inline)
       0   0.0% 100.0%       45 100.0% std::__sort (inline)
       0   0.0% 100.0%       37  82.2% std::__unguarded_partition_pivot (inline)
       0   0.0% 100.0%       45 100.0% std::sort (inline)
```

After using int instead of unsigned int:

```
$ CPUPROFILE=prof.data ./example01
L:1048576
N:65536
Sort time: 422ms (1262836 comparisons)
PROFILE: interrupts/evictions/bytes = 42/0/2272

$ pprof --text ./example01 prof.data
Using local file ./example01.
Using local file prof.data.
Total: 42 samples
      36  85.7%  85.7%       36  85.7% std::__unguarded_partition (inline)
       6  14.3% 100.0%        6  14.3% std::__unguarded_insertion_sort (inline)
       0   0.0% 100.0%       42 100.0% __libc_start_call_main
       0   0.0% 100.0%       42 100.0% __libc_start_main_impl
       0   0.0% 100.0%       42 100.0% _start
       0   0.0% 100.0%       42 100.0% main
       0   0.0% 100.0%        6  14.3% std::__final_insertion_sort (inline)
       0   0.0% 100.0%       36  85.7% std::__introsort_loop
       0   0.0% 100.0%       42 100.0% std::__sort (inline)
       0   0.0% 100.0%       36  85.7% std::__unguarded_partition_pivot (inline)
       0   0.0% 100.0%       42 100.0% std::sort (inline)
```