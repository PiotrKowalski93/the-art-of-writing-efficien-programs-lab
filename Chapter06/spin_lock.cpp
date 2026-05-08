#include <time.h>
#include <atomic>

class Spinlock{
    public:
        void lock(){
            // (1) flag_.load(std::memory_order_relaxed) - first, fast check if flag is locked
            // (2) flag_.exchange(1, std::memory_order_acquire) - expensive, exchange = locking
            // if (1) is 1 then exchange (2) is not done, faster

            // if flag_.exchange(1, std::memory_order_acquire) returns 0, and load returns 0, then we locked
            // we use relaxed during load, beacuse it is more efficient, we do not need mem sync
            for (int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); ++i)
            {
                if(i == 8){
                    lock_sleep();
                    i = 0;
                }
            }
        }

        void unlock(){
            flag_.exchange(0, std::memory_order_release);
        }

    private:
        // 0 - free, 1 - locked
        std::atomic<unsigned int> flag_;

        void lock_sleep(){
            static const timespec ns = {0, 1,};
            nanosleep(&ns, NULL);
        }
};