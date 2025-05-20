#include "object.h"
#include <cmath>
#include <array>
#include <C:\Users\Nick-Uni\Desktop\Sim Methods\GLFW\glfw3.h>

float PI = 3.141592653589;

Object::Object()
    : position({0.0f, 0.0f}), velocity({0.0f, 0.0f}), radius(1.0f), mass(1.0f){}

Object::Object(std::array<float, 2> position, std::array<float, 2> velocity, float radius, float mass)
    : position(position), velocity(velocity), radius(radius), mass(mass){}

void Object::accelerate(float x, float y, float dt){
    velocity[0] += x * dt;
    velocity[1] += y * dt;
}

void Object::updatePos(float dt){
    position[0] += velocity[0] * dt;
    position[1] += velocity[1] * dt;
    // camera_position = position;
}

void Object::DrawCircle() {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(position[0], position[1]);
    for (int i = 0; i <= res; ++i) {
        float angle = 2.0f * PI * (static_cast<float>(i) / res);
        float x = position[0] + cos(angle) * radius;
        float y = position[1] + sin(angle) * radius;
        glVertex2d(x, y);
    }
    glEnd();
}