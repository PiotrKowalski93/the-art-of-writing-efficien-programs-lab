```bash
$ g++ -std=c++20 -O3 -Wall -Wextra stack.cpp -o stack

$ g++ -std=c++20 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include mutex_stack.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o mutex_stack

$ g++ -std=c++20 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include readwrite_stack.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o readwrite_stack
```