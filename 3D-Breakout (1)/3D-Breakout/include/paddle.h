#ifndef PADDLE_H
#define PADDLE_H

#include <glm/glm.hpp>

class Paddle {
public:
    glm::vec3 position;
    glm::vec3 size;
    float speed;
    
    Paddle(glm::vec3 pos, glm::vec3 sz, float spd);
    
    void moveLeft(float deltaTime, float minX);
    void moveRight(float deltaTime, float maxX);
    void reset(glm::vec3 pos);
};

#endif