#include <stack>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <optional>
#include <benchmark/benchmark.h>

// stack with read-write lock
template <typename T> class readwrite_stack {
    std::stack<T> _stack;
    mutable std::shared_mutex _l;

public:
    readwrite_stack() = default;

    void push(const T& obj){
        std::unique_lock g(_l);
        _stack.push(obj);
    }

    std::optional<T> pop(){
        std::unique_lock g(_l);

        if(_stack.empty()){
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> result(std::move(_stack.top()));
            _stack.pop();
            return result;
        }
    }

    std::optional<T> top() const {
        std::shared_lock g(_l);

        if(_stack.empty()){
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> result(_stack.top());
            return result;
        }
    }
};

readwrite_stack<int> s;
void BM_readwrite_stackk(benchmark::State& state){
    const size_t N = state.range(0);

    for(auto _ : state){
        for (size_t i = 0; i < N; ++i) s.push(i);
        for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s.pop());
    }

    state.SetItemsProcessed(state.iterations()*N);
}

void BM_rw_top(benchmark::State& state)
{
    const size_t N = state.range(0);
    for (size_t i = 0; i < N; ++i) s.push(i);

    for(auto _ : state)
    {
        benchmark::DoNotOptimize(s.top());

        for(int i = 0; i < 1000; i++)
        {
            for(int j = 0; j < 1000; j++){
                benchmark::DoNotOptimize(j);
            }
            benchmark::DoNotOptimize(i);
        }
    }
}

BENCHMARK(BM_rw_top)->Arg(64)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8);

BENCHMARK(BM_readwrite_stackk)->Arg(1024)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8);

BENCHMARK_MAIN();