#include <stack>
#include <mutex>
#include <iostream>
#include <optional>
#include <benchmark/benchmark.h>

using namespace std;

// stack with mutex lock
// Encapsulation by composition - by that we ensure that there is no use of non-thread safe methods
template <typename T> class mutex_stack {
    std::stack<T> _stack;
    mutable std::mutex _l;

public:
    mutex_stack() = default;

    void push(const T& obj){
        std::lock_guard g(_l);
        _stack.push(obj);
    }

    std::optional<T> pop(){
        std::lock_guard g(_l);

        if(_stack.empty()){
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> result(std::move(_stack.top()));
            _stack.pop();
            return result;
        }
    }

    std::optional<T> top() const {
        std::lock_guard g(_l);

        if(_stack.empty()){
            return std::optional<T>(std::nullopt);
        } else {
            std::optional<T> result(_stack.top());
            return result;
        }
    }
};

mutex_stack<int> s;
void BM_mutex_stack(benchmark::State& state){
    const size_t N = state.range(0);

    for(auto _ : state){
        for (size_t i = 0; i < N; ++i) s.push(i);
        for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s.pop());
    }

    state.SetItemsProcessed(state.iterations()*N);
}

void BM_mutex_top(benchmark::State& state)
{
    const size_t N = state.range(0);
    for (size_t i = 0; i < N; ++i) s.push(i);

    for(auto _ : state)
    {
        benchmark::DoNotOptimize(s.top());

        for(int i=0;i<1000;i++)
            benchmark::DoNotOptimize(i);
    }
}

BENCHMARK(BM_mutex_top)->Arg(64)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8);

BENCHMARK(BM_mutex_stack)->Arg(1024)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8);

BENCHMARK_MAIN();