#include <stdlib.h>
#include <string.h>
#include <iostream>

#define MCA_START __asm volatile("# LLVM-MCA-BEGIN");
#define MCA_END __asm volatile("# LLVM-MCA-END");

#include "benchmark/benchmark.h"

void BM_add_multiply(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);

    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;

        for (size_t i = 0; i < N; ++i) 
        {
            a1 += p1[i] * p2[i];
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_branch_not_predicted(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);
    std::vector<int> c1(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_branch_predicted(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);
    std::vector<int> c1(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() >= 0;
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_false_branch(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);

    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    int* b2 = c2.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i] || b2[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_false_branch_temp_var(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);

    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    int* b2 = c2.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            const bool b3 = b1[i] || b2[i];
            if (b3) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_false_branch_conditions_in_array(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);

    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);

    std::vector<int> c(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    std::vector<int> c3(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
        c3[i] = c1[i] || c2[i];
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b3 = c3.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b3[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_false_branch_sum(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);

    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    int* b2 = c2.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i] + b2[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_false_branch_bitwise(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);

    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }
    
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    int* b2 = c2.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i] | b2[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_add_multiply_unrolled(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<unsigned long> v2(N);

    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();

    for (auto _ : state) {
        unsigned long a1 = 0;
        unsigned long a2 = 0;

        for (size_t i = 0; i < N; i += 16) {
            a1 += p1[i     ] * p2[i     ]
               +  p1[i +  1] * p2[i +  1]
               +  p1[i +  2] * p2[i +  2]
               +  p1[i +  3] * p2[i +  3]
               +  p1[i +  4] * p2[i +  4]
               +  p1[i +  5] * p2[i +  5]
               +  p1[i +  6] * p2[i +  6]
               +  p1[i +  7] * p2[i +  7]
               +  p1[i +  8] * p2[i +  8]
               +  p1[i +  9] * p2[i +  9]
               +  p1[i + 10] * p2[i + 10]
               +  p1[i + 11] * p2[i + 11]
               +  p1[i + 12] * p2[i + 12]
               +  p1[i + 13] * p2[i + 13]
               +  p1[i + 14] * p2[i + 14]
               +  p1[i + 15] * p2[i + 15];
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N*state.iterations());
}

void BM_branched(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<int> c1(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        c1[i] = rand() & 0x1;
    }
    
    unsigned long* p1 = v1.data();
    int* b1 = c1.data();

    for (auto _ : state) {
        unsigned long a1 = 0;
        unsigned long a2 = 0;
        
        for (size_t i = 0; i < N; ++i) {
            if (b1[i]) {
                a1 += p1[i];
            } else {
                a2 += p1[i];
            }
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_branchless(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<int> c1(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        c1[i] = rand() & 0x1;
    }
    
    unsigned long* p1 = v1.data();
    int* b1 = c1.data();

    for (auto _ : state) {
        unsigned long a1 = 0;
        unsigned long a2 = 0;
        
        unsigned long* a[2] = { &a1, &a2 };
        
        for (size_t i = 0; i < N; ++i) {
            a[b1[i]] += p1[i];
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

void BM_branchless_question_mark_operator(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<int> c1(N);
    
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        c1[i] = rand() & 0x1;
    }
    
    unsigned long* p1 = v1.data();
    int* b1 = c1.data();

    for (auto _ : state) {
        unsigned long a1 = 0;
        unsigned long a2 = 0;
        
        for (size_t i = 0; i < N; ++i) {
            (b1[i] ? a1 : a2) += p1[i];
        }

        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
}

// ------------ First measures
// BENCHMARK(BM_add_multiply)->Arg(1<<22);
// BENCHMARK(BM_branch_not_predicted)->Arg(1<<22);
// BENCHMARK(BM_branch_predicted)->Arg(1<<22);
// BENCHMARK(BM_false_branch)->Arg(1<<22);

// ------------ Complex conditions optimization
// BENCHMARK(BM_branch_predicted)->Arg(1<<22);
// BENCHMARK(BM_false_branch)->Arg(1<<22);
// BENCHMARK(BM_false_branch_temp_var)->Arg(1<<22);
// BENCHMARK(BM_false_branch_conditions_in_array)->Arg(1<<22);
// BENCHMARK(BM_false_branch_sum)->Arg(1<<22);
// BENCHMARK(BM_false_branch_bitwise)->Arg(1<<22);

// ------------ Loop Unrolling
// BENCHMARK(BM_add_multiply)->Arg(1<<22);
// BENCHMARK(BM_add_multiply_unrolled)->Arg(1<<22);

// ------------ Branchless selection
BENCHMARK(BM_branched)->Arg(1<<22);
BENCHMARK(BM_branchless)->Arg(1<<22);
BENCHMARK(BM_branchless_question_mark_operator)->Arg(1<<22);

BENCHMARK_MAIN();