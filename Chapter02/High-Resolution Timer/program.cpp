#include <time.h>
#include <math.h>
#include <iostream>
#include <unistd.h>
#include <future>

double duration(timespec a, timespec b) {
    return a.tv_sec - b.tv_sec + 1e-9*(a.tv_nsec - b.tv_nsec);
}

int main() {

    // Computations time:
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);

        constexpr double X = 1e6;
        double s = 0;
        for (double x = 0; x < X; x += 0.1) s += sin(x);
        
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);

        std::cout << "Computation time: " << std::endl;
        std::cout << "Real time: " << duration(rt1, rt0) << "s, " 
                << "CPU time: " << duration(ct1, ct0) << "s, "
                << "Thread time: " << duration(tt1, tt0) << "s, "
                << "Result " << s << std::endl;
        std::cout << std::endl;
    }
 
    // Idle time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);

        sleep(1);
        
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);

        std::cout << "Idle time: " << std::endl;
        std::cout << "Real time: " << duration(rt1, rt0) << "s, " 
                << "CPU time: " << duration(ct1, ct0) << "s, "
                << "Thread time: " << duration(tt1, tt0) << "s" << std::endl;
        std::cout << std::endl;
    }

    // Thread time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);

        constexpr double X = 1e6;
        double s = 0;
        auto t = std::async(std::launch::async, [&]{
            for (double x = 0; x < X; x += 0.1) s += sin(x);
        });
        t.wait();

        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);

        std::cout << "Thread time: " << std::endl;
        std::cout << "Real time: " << duration(rt1, rt0) << "s, " 
                << "CPU time: " << duration(ct1, ct0) << "s, "
                << "Thread time: " << duration(tt1, tt0) << "s" << std::endl;
        std::cout << std::endl;
    }

    // Multiple threads
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);

        constexpr double X = 1e6;
        double s1 = 0, s2 = 0;
        auto t = std::async(std::launch::async, [&]{
            for (double x = 0; x < X; x += 0.1) s1 += sin(x);
        });
        for (double x = 0; x < X; x += 0.1) s2 += sin(x);
        t.wait();

        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);

        std::cout << "Multiple threads time: " << std::endl;
        std::cout << "Real time: " << duration(rt1, rt0) << "s, " 
                << "CPU time: " << duration(ct1, ct0) << "s, "
                << "Thread time: " << duration(tt1, tt0) << "s, "
                << "Results: " << s1 << ", " << s2 << std::endl;
        std::cout << std::endl;
    }

}