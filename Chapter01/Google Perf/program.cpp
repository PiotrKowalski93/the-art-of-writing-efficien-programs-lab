#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "compare_1.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::cout;
using std::endl;
using std::minstd_rand;
using std::unique_ptr;
using std::vector;

bool compare(const char* s1, const char* s2, unsigned int l);

int main() {
    constexpr unsigned int L = 1 << 18, N = 1 << 14;

    cout << "L:" << L << endl;
    cout << "N:" << N << endl;

    unique_ptr<char[]> s(new char[L]);
    vector<const char*> vs(N);

    //  prep string
    minstd_rand r_gen;
    ::memset(s.get(), 'a', N*sizeof(char));

    for(unsigned int i = 0; i < L/1024; ++i){
        s[r_gen() % (L-1)] = 'a' + (r_gen() % ('z' - 'a' + 1));
    }
    s[L-1] = 0;
    for(unsigned int i = 0; i < N; ++i){
        vs[i] = &s[r_gen() % (L-1)];
    }

    size_t count = 0;
    system_clock::time_point t1 = system_clock::now();

    // Adapter Pattern
    std::sort(vs.begin(), vs.end(), 
        [&](const char* a, const char* b){
            ++count;
            return compare(a, b);
        });

    system_clock::time_point t2 = system_clock::now();
    cout << "Sort time: " << duration_cast<milliseconds>(t2 - t1).count() << "ms (" << count << " comparisons)" << endl;
}