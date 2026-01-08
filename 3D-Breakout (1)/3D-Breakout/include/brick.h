#ifndef BRICK_H
#define BRICK_H

#include <glm/glm.hpp>

class Brick {
public:
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    bool destroyed;
    
    Brick(glm::vec3 pos, glm::vec3 sz, glm::vec3 col);
    
    void destroy();
    void reset();
};

#endif