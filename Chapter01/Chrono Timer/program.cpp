#include <chrono>
#include <iostream>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

int main() {

    // Work
    auto t0 = system_clock::now();
    // Work

    auto t1 = system_clock::now();
    auto delta_t = duration_cast<milliseconds>(t1-t0);
    std::cout << "Time: " << delta_t.count() << std::endl;
}