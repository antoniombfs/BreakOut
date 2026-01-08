#include "paddle.h"

Paddle::Paddle(glm::vec3 pos, glm::vec3 sz, float spd) 
    : position(pos), size(sz), speed(spd) {}

void Paddle::moveLeft(float deltaTime, float minX) {
    position.x -= speed * deltaTime;
    if (position.x - size.x / 2.0f < minX) {
        position.x = minX + size.x / 2.0f;
    }
}

void Paddle::moveRight(float deltaTime, float maxX) {
    position.x += speed * deltaTime;
    if (position.x + size.x / 2.0f > maxX) {
        position.x = maxX - size.x / 2.0f;
    }
}

void Paddle::reset(glm::vec3 pos) {
    position = pos;
}