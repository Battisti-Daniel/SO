#include <chrono>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

class Timer {
public:
    Timer() { start = clock.now(); }
    double GetElapsed() {
        auto end = clock.now();
        auto duration = end - start;
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() * 1.e-9;
    }
private:
    std::chrono::steady_clock clock;
    std::chrono::steady_clock::time_point start;
};

void BusyWait(int ms) {
    auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
    while (std::chrono::steady_clock::now() < end) {}
}

void FastMeasure(size_t mb) {
    printf("Busy waiting to raise the CPU frequency...\n");
    BusyWait(500);

    const size_t bufSize = mb * 1024ULL * 1024ULL;
    const int iterationCount = 100;

    {
        Timer timer;
        for (int i = 0; i < iterationCount; ++i) {
            int* p = new int[bufSize / sizeof(int)];
            delete[] p;
        }
        printf("%1.4f s to allocate %zu MB %d times.\n", timer.GetElapsed(), mb, iterationCount);
    }

    {
        Timer timer;
        double deleteTime = 0.0;
        for (int i = 0; i < iterationCount; ++i) {
            int* p = new int[bufSize / sizeof(int)];
            Timer deleteTimer;
            delete[] p;
            deleteTime += deleteTimer.GetElapsed();
        }
        printf("%1.4f s to allocate %zu MB %d times (%1.4f s to delete).\n", timer.GetElapsed(), mb, iterationCount, deleteTime);
    }

    {
        int* p = new int[bufSize / sizeof(int)]();
        {
            Timer timer;
            for (int i = 0; i < iterationCount; ++i) {
                memset(p, 1, bufSize);
            }
            printf("%1.4f s to write %zu MB %d times.\n", timer.GetElapsed(), mb, iterationCount);
        }
        {
            Timer timer;
            long long sum = 0;
            for (int i = 0; i < iterationCount; ++i) {
                for (size_t index = 0; index < bufSize / sizeof(int); ++index) {
                    sum += p[index];
                }
            }
            printf("%1.4f s to read %zu MB %d times, sum = %lld.\n", timer.GetElapsed(), mb, iterationCount, sum);
        }
        delete[] p;
    }

    {
        Timer timer;
        double deleteTime = 0.0;
        for (int i = 0; i < iterationCount; ++i) {
            int* p = new int[bufSize / sizeof(int)];
            memset(p, 1, bufSize);
            Timer deleteTimer;
            delete[] p;
            deleteTime += deleteTimer.GetElapsed();
        }
        printf("%1.4f s to allocate and write %zu MB %d times (%1.4f s to delete).\n", timer.GetElapsed(), mb, iterationCount, deleteTime);
    }

    {
        Timer timer;
        long long sum = 0;
        for (int i = 0; i < iterationCount; ++i) {
            int* p = new int[bufSize / sizeof(int)];
            for (size_t index = 0; index < bufSize / sizeof(int); ++index) {
                sum += p[index];
            }
            delete[] p;
        }
        printf("%1.4f s to allocate and read %zu MB %d times, sum = %lld.\n", timer.GetElapsed(), mb, iterationCount, sum);
    }
}

int main(int argc, char* argv[]) {
    size_t mb = (argc >= 2) ? std::stoull(argv[1]) : 32ULL;
    FastMeasure(mb);
    return 0;
}
