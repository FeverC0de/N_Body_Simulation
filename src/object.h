#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <cmath>
#include <memory>
#include <array>

class Object{
public:
        std::array<float, 2> position;
        std::array<float, 2> velocity;
        // std::array<float, 2> acceleration;

        float radius;
        float mass;
        int res = 5;

        Object();
        Object(std::array<float,2> position, std::array<float,2> velocity, float radius, float mass);

        void accelerate(float x, float y, float dt);
        void updatePos(float dt);
        void DrawCircle();
};

#endif