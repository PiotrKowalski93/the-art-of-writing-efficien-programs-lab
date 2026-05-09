#include <time.h>
#include <atomic>

// flag_ == nullptr ==> taken
// flag_ != nullptr ==> free

template <typename T>
class PtrSpinLock{
    public:
    explicit PtrSpinLock(T* flag) : flag_(flag){}

    T* lock(){
        for (int i = 0;
            flag_.load(std::memory_order_relaxed) == nullptr                                // is free, then try to aquire
                || (flag = flag_.exchange(NULL, std::memory_order_acquire)) == nullptr;     // could not aquire, try again or wait nanosecond 
            ++i)
        {
            if(i == 8){
                nanosleep(&ns, NULL);
                i = 0;
            }
        }
        return saved_flag_ = flag;  // aquired, return pointer to change
    }

    void unlock(){
        flag_.exchange(saved_flag_, std::memory_order_release); // assing ptr, will be not null == free
    }

    private:
    static const timespec ns = {0, 1,};

    std::atomic<T*> flag_;
    T* saved_flag_ = nullptr;
};