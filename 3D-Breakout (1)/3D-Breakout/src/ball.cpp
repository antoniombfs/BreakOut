#include "ball.h"

Ball::Ball(glm::vec3 pos, glm::vec3 vel, float r) 
    : position(pos), velocity(vel), radius(r) {}

void Ball::update(float dt) {
    position += velocity * dt;
}

void Ball::reset(glm::vec3 pos, glm::vec3 vel) {
    position = pos;
    velocity = vel;
}

void Ball::reverseX() {
    velocity.x = -velocity.x;
}

void Ball::reverseY() {
    velocity.y = -velocity.y;
}

void Ball::addSpin(float spin) {
    velocity.x += spin * 10.0f; 
    
    if (velocity.x > 12.0f) velocity.x = 12.0f;
    if (velocity.x < -12.0f) velocity.x = -12.0f;
}