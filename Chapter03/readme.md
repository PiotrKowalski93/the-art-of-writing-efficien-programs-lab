```
$ export GBENCH_DIR=/home/kowal/Desktop/Git/google-benchmark
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include superscalar.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o superscalar
```