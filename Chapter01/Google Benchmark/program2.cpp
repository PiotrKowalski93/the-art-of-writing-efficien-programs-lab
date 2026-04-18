#include <benchmark/benchmark.h>
#include <memory>
#include <cstring>

#include "compare_int.h"
#include "compare_uint.h"
#include "compare_uint_l.h"

using std::unique_ptr;

extern bool compare_int(const char* s1, const char* s2);
extern bool compare_uint(const char* s1, const char* s2);
extern bool compare_uint_l(const char* s1, const char* s2);

void BM_loop_int(benchmark::State& state){
    const unsigned int N = state.range(0);
    unique_ptr<char[]> s(new char[2*N]);

    ::memset(s.get(), 'a', 2*N*sizeof(char));

    s[2*N-1] = 0;
    const char* s1 = s.get();
    const char* s2 = s1 + N;

    for(auto _ : state){
        benchmark::DoNotOptimize(compare_int(s1,s2));
    }
    state.SetItemsProcessed(N*state.iterations());
}

BENCHMARK(BM_loop_int) -> Arg(1<<20);
BENCHMARK_MAIN();