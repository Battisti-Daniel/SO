#include "pgm.hpp"
#include "common.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <fifo_path> <imagem.pgm>\n";
        return 1;
    }

    const char* fifo = argv[1];
    const char* imgpath = argv[2];

    if (mkfifo(fifo, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar FIFO");
        return 1;
    }

    PGM img;
    if (read_pgm(imgpath, img) != 0) {
        std::cerr << "Erro ao ler imagem\n";
        return 1;
    }

    Header header{
        img.w,
        img.h,
        img.maxv,
        0,
        0,
        0
    };

    int fd = open(fifo, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO");
        return 1;
    }

    write(fd, &header, sizeof(header));
    write(fd, img.data.data(), img.data.size());
    close(fd);

    std::cout << "Imagem enviada.\n";
    return 0;
}
