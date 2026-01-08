#ifndef BALL_H
#define BALL_H

#include <glm/glm.hpp>

class Ball {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    
    Ball(glm::vec3 pos, glm::vec3 vel, float r);
    
    void update(float dt);
    void reset(glm::vec3 pos, glm::vec3 vel);
    void reverseX();
    void reverseY();
    void addSpin(float spin);
};

#endif