#include "brick.h"

Brick::Brick(glm::vec3 pos, glm::vec3 sz, glm::vec3 col) 
    : position(pos), size(sz), color(col), destroyed(false) {}

void Brick::destroy() {
    destroyed = true;
}

void Brick::reset() {
    destroyed = false;
}