#include <stack>
#include <mutex>
#include <iostream>
#include <optional>
#include <benchmark/benchmark.h>

using namespace std;

// stack with mutex lock
// Encapsulation by composition - by that we ensure that there is no use of non-thread safe methods
template <typename T> class mt_stack {
    std::stack<T> _stack;
    std::mutex _l;

public:
    mt_stack() = default;

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
};

mt_stack<int> s;
void BM_mt_stack(benchmark::State& state){
    const size_t N = state.range(0);

    for(auto _ : state){
        for (size_t i = 0; i < N; ++i) s.push(i);
        for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(s.pop());
    }

    state.SetItemsProcessed(state.iterations()*N);
}

BENCHMARK(BM_mt_stack)->Arg(1024)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8);

BENCHMARK_MAIN();