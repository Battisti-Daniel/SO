#include "pgm.hpp"
#include <fstream>
#include <iostream>

int read_pgm(const std::string& path, PGM& img){
    std::ifstream f(path, std::ios::binary);
    if(!f) return -1;

    std::string magic;

    f>>magic;
    if (magic != "P5") return -2;

    std::cout << img.w << " " << img.h << " " << img.maxv << "\n";

    f >> img.w >> img.h >> img.maxv;
    f.ignore(1);

    img.data.resize(img.w * img.h);
    f.read((char*)img.data.data(), img.data.size());
    return 0;
}

int write_pgm(const std::string& path, const PGM& img) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return -1;

    f << "P5\n" << img.w << " " << img.h << "\n" << img.maxv << "\n";
    f.write((const char*)img.data.data(), img.data.size());
    return 0;
}