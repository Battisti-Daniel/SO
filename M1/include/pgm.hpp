#pragma once
#include <string>
#include <vector>

struct PGM{
    int w, h, maxv;
    std::vector<unsigned char> data;
};

int read_pgm(const std::string& path, PGM& img);
int write_pgm(const std::string& path, const PGM& img);