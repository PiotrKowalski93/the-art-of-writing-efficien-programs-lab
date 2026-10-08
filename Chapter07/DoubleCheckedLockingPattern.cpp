#include <atomic>
#include <stddef.h>
#include <mutex>

std::atomic<size_t> nmax{0};
std::mutex _l;

void atomic_max(size_t n){
    if(n > nmax.load(std::memory_order_relaxed)){
        std::lock_guard L(_l);
         if(n > nmax.load(std::memory_order_relaxed)){
            nmax.store(n, std::memory_order_relaxed);
        }
    }
}

