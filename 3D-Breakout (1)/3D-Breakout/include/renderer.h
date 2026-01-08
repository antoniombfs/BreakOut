#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <vector>

class Renderer {
public:
    GLuint cubeVAO;
    GLuint sphereVAO;
    int sphereVertexCount;
    
    Renderer();
    ~Renderer();
    
    void init();
    void cleanup();
    
private:
    GLuint cubeVBO;
    GLuint sphereVBO;
    
    void createCube();
    void createSphere(float radius, int sectors, int stacks);
};

#endif