#pragma once
#include <semaphore.h>
#include <pthread.h>

#define MODE_NEG   0
#define MODE_SLICE 1
#define FIFO_PATH "/tmp/imgpipe"

struct Header {
    int w, h, maxv;
    int mode;
    int t1, t2;
};

struct Task {
    int row_start;
    int row_end;
};


