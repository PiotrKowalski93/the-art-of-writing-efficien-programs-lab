```bash
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include random_mem_access.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o random
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include sequential_mem_access.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o sequential
```