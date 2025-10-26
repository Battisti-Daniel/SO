#include "pgm.hpp"
#include "common.hpp"
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <queue>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define QMAX 128

std::queue<Task> q;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_items, sem_space;

PGM g_in, g_out;
int g_mode, g_t1, g_t2, g_nthreads;
bool done = false;

void apply_negative_block(int rs, int re) {
    for (int y = rs; y < re; y++) {
        for (int x = 0; x < g_in.w; x++) {
            int idx = y * g_in.w + x;
            g_out.data[idx] = 255 - g_in.data[idx];
        }
    }
}

void apply_slice_block(int rs, int re, int t1, int t2) {
    for (int y = rs; y < re; y++) {
        for (int x = 0; x < g_in.w; x++) {
            int idx = y * g_in.w + x;
            unsigned char p = g_in.data[idx];
            g_out.data[idx] = (p <= t1 || p >= t2) ? 255 : p;
        }
    }
}

void* worker_thread(void* arg) {
    while (true) {
        sem_wait(&sem_items);
        pthread_mutex_lock(&q_lock);
        if (q.empty()) {
            pthread_mutex_unlock(&q_lock);
            continue; 
        }
        Task t = q.front();
        q.pop();
        pthread_mutex_unlock(&q_lock);
        sem_post(&sem_space);

        if (t.row_start < 0) break;

        if (g_mode == MODE_NEG)
            apply_negative_block(t.row_start, t.row_end);
        else
            apply_slice_block(t.row_start, t.row_end, g_t1, g_t2);
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Uso: " << argv[0] << " <fifo_path> <saida.pgm> <negativo|slice> [t1 t2] [nthreads]\n";
        return 1;
    }

    const char* fifo = argv[1];
    const char* outpath = argv[2];
    std::string mode = argv[3];

    if (mode == "negativo") {
        g_mode = MODE_NEG;
        g_nthreads = (argc >= 5) ? atoi(argv[4]) : 4;
    } else if (mode == "slice") {
        g_mode = MODE_SLICE;
        g_t1 = atoi(argv[4]);
        g_t2 = atoi(argv[5]);
        g_nthreads = (argc >= 7) ? atoi(argv[6]) : 4;
    } else {
        std::cerr << "Modo inválido\n";
        return 1;
    }

    mkfifo(fifo, 0666);
    int fd = open(fifo, O_RDONLY);
    Header hdr;
    read(fd, &hdr, sizeof(hdr));
    g_in.w = hdr.w;
    g_in.h = hdr.h;
    g_in.maxv = hdr.maxv;
    g_in.data.resize(g_in.w * g_in.h);
    read(fd, g_in.data.data(), g_in.data.size());
    close(fd);

    g_out = g_in;

    sem_init(&sem_items, 0, 0);
    sem_init(&sem_space, 0, QMAX);

    std::vector<pthread_t> threads(g_nthreads);
    for (auto &t : threads)
        pthread_create(&t, nullptr, worker_thread, nullptr);

    int step = g_in.h / g_nthreads;
    for (int i = 0; i < g_nthreads; i++) {
        Task task{i * step, (i == g_nthreads - 1) ? g_in.h : (i + 1) * step};
        sem_wait(&sem_space);
        pthread_mutex_lock(&q_lock);
        q.push(task);
        pthread_mutex_unlock(&q_lock);
        sem_post(&sem_items);
    }

    for (int i = 0; i < g_nthreads; i++) {
        Task t{-1, -1};
        sem_wait(&sem_space);
        pthread_mutex_lock(&q_lock);
        q.push(t);
        pthread_mutex_unlock(&q_lock);
        sem_post(&sem_items);
    }

    for (auto &t : threads)
        pthread_join(t, nullptr);

    write_pgm(outpath, g_out);
    std::cout << "Processamento concluído: " << outpath << "\n";
    return 0;
}
