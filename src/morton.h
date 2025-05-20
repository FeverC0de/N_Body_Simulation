#ifndef MORTON_H
#define MORTON_H

#include <array>
#include <stdint.h>


class Morton{
    public:

    std::array<int, 2> toInt(std::array<float, 2> pos);

    int toGrid(int grid_size, float coord, float  min, float max);

    uint64_t encode(std::array<float, 2> pos, float min, float max);
    int decode(std::array<int, 2> pos);

    Morton();

};

#endif