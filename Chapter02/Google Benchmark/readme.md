### Setup Google Benchamrk

Clone and build:

``` 
$ git clone https://github.com/google/benchmark.git google-benchmark
$ cd google-benchmark
$ cmake -E make_directory build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
```

Setup path:
```
$ export GBENCH_DIR=/home/kowal/Desktop/Git/google-benchmark
```

Build with proper linking:
```
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include program.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o example
```

Run:
```
$ ./example --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```
