#ifndef GAME_H
#define GAME_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

#include "shader.h"
#include "renderer.h"
#include "ball.h"
#include "paddle.h"
#include "brick.h"
#include "..\src\Model3D.hpp"

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_LOSE
};

class Game {
public:
    GameState state;
    bool keys[1024];
    unsigned int width, height;

    Game(unsigned int width, unsigned int height);
    ~Game();
    
    void init();
    void processInput(float dt);
    void update(float dt);
    void render();
    void processMouseMovement(float xpos, float ypos);
    void updateResolution(unsigned int w, unsigned int h);

private:
    void reset();
    void createBricks();
    void checkCollisions();
    bool checkBallPaddleCollision();
    bool checkBallBrickCollision(Brick* b);
    void updateCamera();
    void loadArcadeModel();
    void renderUI(); 

    Renderer* renderer;
    Shader* shader;
    Ball* ball;
    Paddle* paddle;
    std::vector<Brick*> bricks;
    Model3D* arcadeModel;

    unsigned int score;
    int lives;
    
    bool useArcadeModel;
    bool firstMouse;
    bool mouseCaptured;
    
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraYaw;
    float cameraPitch;
    float lastMouseX, lastMouseY;
    
    glm::vec3 bricksPlanePosition;
    glm::vec3 bricksPlaneRotation;
    float gameScale;
    glm::vec3 arcadePosition;
    glm::vec3 arcadeRotation;
    glm::vec3 arcadeScale;
    glm::mat4 projection;
    glm::mat4 view;

    float gameLimitLeft;
    float gameLimitRight;
    float gameLimitTop;
    float gameLimitBottom;
};

#endif