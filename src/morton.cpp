#include "morton.h"
#include <limits.h>
#include <stdint.h>
#include <iostream>





uint64_t Morton::encode(std::array<float, 2> pos, float min , float max) {
    uint64_t answer = 0;

    int grid_size = 1 << 25;
    unsigned int x = toGrid(grid_size, pos[0], min, max);
    unsigned int y = toGrid(grid_size, pos[1], min, max);

    for(uint64_t i = 0; i < (sizeof(uint64_t)* CHAR_BIT)/2; ++i){
        answer |= ((x & ((uint64_t)1 << i)) << i)
               | ((y & ((uint64_t)1 << i ))<< (i + 1));
    }
    return answer;
}

int Morton::toGrid(int grid_size, float coord, float min, float max){
    unsigned int position = (coord - min) / (max - min) * (grid_size - 1);
    return static_cast<unsigned int>(position);
}

Morton::Morton(){}

