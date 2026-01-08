#include "game.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include "imgui.h" 

const float INITIAL_BALL_SPEED = 12.0f;

const glm::vec3 CFG_GAME_POS    = glm::vec3(0.6540f, -4.9120f, -0.9030f);
const glm::vec3 CFG_GAME_ROT    = glm::vec3(-18.70f, -89.60f, -0.10f);
const float     CFG_GAME_SCALE  = 0.0139f;
const glm::vec3 CFG_CAM_POS     = glm::vec3(0.000f, -4.600f, -0.900f); 

Game::Game(unsigned int width, unsigned int height) 
    : state(GAME_MENU), width(width), height(height), score(0), lives(3), 
      arcadeModel(nullptr), useArcadeModel(true), 
      firstMouse(true), mouseCaptured(true),
      cameraYaw(43.0f), cameraPitch(-25.0f), 
      gameScale(CFG_GAME_SCALE),

      gameLimitLeft(-14.5f),
      gameLimitRight(14.5f),
      
      gameLimitTop(9.5f),
      
      gameLimitBottom(-10.0f)
{
    std::cout << "Game constructor" << std::endl;
    for (int i = 0; i < 1024; i++) keys[i] = false;
    
    bricksPlanePosition = CFG_GAME_POS;
    bricksPlaneRotation = CFG_GAME_ROT;
    arcadePosition = glm::vec3(0.0f, -5.0f, 0.0f);
    arcadeRotation = glm::vec3(0.0f, 180.0f, 0.0f);
    arcadeScale = glm::vec3(1.0f, 1.0f, 1.0f);
}

Game::~Game() {
    delete ball; delete paddle; delete renderer; delete shader;
    for (auto b : bricks) delete b;
    if (arcadeModel) delete arcadeModel;
}

void Game::init() {
    shader = new Shader("shaders/vertex.vert", "shaders/fragment.frag");
    renderer = new Renderer();
    renderer->init();
    
    ball = new Ball(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(8.0f, INITIAL_BALL_SPEED, 0.0f), 0.5f);
    paddle = new Paddle(glm::vec3(0.0f, -8.0f, 0.0f), glm::vec3(4.0f, 0.6f, 1.2f), 20.0f);
    createBricks();
    
    cameraPos = CFG_CAM_POS;
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    lastMouseX = width / 2.0f; lastMouseY = height / 2.0f;
    updateCamera();
    
    projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 200.0f);
    loadArcadeModel(); 
    std::cout << "=== 3D Breakout Ready ===" << std::endl;
}

void Game::loadArcadeModel() {
    if (arcadeModel) { delete arcadeModel; arcadeModel = nullptr; }
    try {
        arcadeModel = new Model3D("arcade/uploads_files_2611707_ArcadeRoom_V1.obj", "arcade/");
        if (arcadeModel->load()) {
            arcadeModel->setupMeshes();
            std::cout << "Model Loaded." << std::endl;
        }
    } catch (const std::exception& e) { std::cerr << "Error loading model: " << e.what() << std::endl; }
}

void Game::updateCamera() {
    glm::vec3 front;
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(front);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Game::processInput(float dt) {
    if (state == GAME_MENU && keys[GLFW_KEY_SPACE]) state = GAME_ACTIVE;
    if ((state == GAME_WIN || state == GAME_LOSE) && keys[GLFW_KEY_R]) reset();
    
    if (state == GAME_ACTIVE) {
        float halfPaddle = paddle->size.x / 2.0f;

        if (keys[GLFW_KEY_A]) {
            paddle->moveLeft(dt, -20.0f);
            if (paddle->position.x - halfPaddle < gameLimitLeft) {
                paddle->position.x = gameLimitLeft + halfPaddle;
            }
        }

        if (keys[GLFW_KEY_D]) {
            paddle->moveRight(dt, 20.0f);
            if (paddle->position.x + halfPaddle > gameLimitRight) {
                paddle->position.x = gameLimitRight - halfPaddle;
            }
        }
    }
    updateCamera();
}

void Game::processMouseMovement(float xpos, float ypos) {
    if (firstMouse) { lastMouseX = xpos; lastMouseY = ypos; firstMouse = false; return; }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos; 
    lastMouseX = xpos; lastMouseY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cameraYaw   += xoffset;
    cameraPitch += yoffset;

    if (cameraPitch > -8.858f)  cameraPitch = -8.858f;
    if (cameraPitch < -43.445f) cameraPitch = -43.445f;

    if (cameraYaw < 7.677f)  cameraYaw = 7.677f;
    if (cameraYaw > 78.484f) cameraYaw = 78.484f;

    updateCamera();
}

void Game::update(float dt) {
    if (state == GAME_ACTIVE) {
        ball->update(dt);
        
        if (ball->position.x <= gameLimitLeft) {
            ball->position.x = gameLimitLeft;
            ball->reverseX();
        }
        else if (ball->position.x >= gameLimitRight) {
            ball->position.x = gameLimitRight;
            ball->reverseX();
        }

        if (ball->position.y >= gameLimitTop) {
            ball->position.y = gameLimitTop;
            ball->reverseY();
        }

        if (ball->position.y <= gameLimitBottom) {
            lives--;
            if (lives <= 0) state = GAME_LOSE;
            else { 
                ball->position = glm::vec3(0.0f, -5.0f, 0.0f); 
                ball->velocity = glm::vec3(8.0f, INITIAL_BALL_SPEED, 0.0f); 
            }
        }
        
        checkCollisions();
        bool allGone = true;
        for (auto b : bricks) if (!b->destroyed) allGone = false;
        if (allGone) state = GAME_WIN;
    }
}

void Game::render() {
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    glm::vec3 neonLightPos = glm::vec3(0.0f, 2.0f, 4.0f); 
    shader->setVec3("lightPos", neonLightPos);

    glm::vec3 neonColor = glm::vec3(0.2f, 0.8f, 1.0f); 
    shader->setVec3("lightColor", neonColor);

    shader->setVec3("viewPos", cameraPos);
    
    if (useArcadeModel && arcadeModel && arcadeModel->loaded()) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, arcadePosition);
        model = glm::rotate(model, glm::radians(arcadeRotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(arcadeRotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(arcadeRotation.z), glm::vec3(0, 0, 1));
        float autoScale = 15.0f / arcadeModel->getMaxDimension();
        model = glm::scale(model, arcadeScale * autoScale);
        
        shader->setMat4("model", model);
        shader->setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        arcadeModel->render(shader->ID);
    }
    
    glm::mat4 gameBase = glm::mat4(1.0f);
    gameBase = glm::translate(gameBase, bricksPlanePosition);
    gameBase = glm::rotate(gameBase, glm::radians(bricksPlaneRotation.y), glm::vec3(0, 1, 0));
    gameBase = glm::rotate(gameBase, glm::radians(bricksPlaneRotation.x), glm::vec3(1, 0, 0));
    gameBase = glm::rotate(gameBase, glm::radians(bricksPlaneRotation.z), glm::vec3(0, 0, 1));
    gameBase = glm::scale(gameBase, glm::vec3(gameScale));

    shader->setInt("useTexture", 0);
    shader->setVec3("material.diffuse", 0.0f, 0.0f, 0.0f);

    shader->setVec3("material.ambient", 0.3f, 0.1f, 0.4f);
    shader->setVec3("material.specular", 1.0f, 1.0f, 1.0f);

    shader->setFloat("material.shininess", 64.0f);
    
    glm::mat4 m;
    
    m = glm::translate(gameBase, paddle->position); m = glm::scale(m, paddle->size);
    shader->setMat4("model", m); shader->setVec3("objectColor", 0.3f, 0.7f, 1.0f);
    glBindVertexArray(renderer->cubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);

    m = glm::translate(gameBase, ball->position); m = glm::scale(m, glm::vec3(ball->radius));
    shader->setMat4("model", m); shader->setVec3("objectColor", 1.0f, 1.0f, 1.0f);
    glBindVertexArray(renderer->sphereVAO); glDrawArrays(GL_TRIANGLES, 0, renderer->sphereVertexCount);

    for (auto b : bricks) {
        if (!b->destroyed) {
            m = glm::translate(gameBase, b->position); m = glm::scale(m, b->size);
            shader->setMat4("model", m); shader->setVec3("objectColor", b->color);
            glBindVertexArray(renderer->cubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    renderUI();
}

void Game::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SCORE: %05d", score);
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "LIVES: %d", lives);
    ImGui::End();

    if (state != GAME_ACTIVE) {
        ImGui::SetNextWindowPos(ImVec2(width/2.0f - 150, height/2.0f - 50));
        ImGui::Begin("MenuState", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(2.0f);
        
        if (state == GAME_MENU) {
            ImGui::Text("PRESS SPACE TO START");
        } 
        else if (state == GAME_WIN) {
            ImGui::TextColored(ImVec4(0,1,0,1), "YOU WIN!");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Text("Press R to Restart");
        } 
        else if (state == GAME_LOSE) {
            ImGui::TextColored(ImVec4(1,0,0,1), "GAME OVER");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Text("Press R to Restart");
        }
        ImGui::End();
    }
}

void Game::updateResolution(unsigned int w, unsigned int h) { width = w; height = h; projection = glm::perspective(glm::radians(45.0f), (float)w/(float)h, 0.1f, 200.0f); }
void Game::reset() { score = 0; lives = 3; state = GAME_MENU; ball->position = glm::vec3(0.0f, -5.0f, 0.0f); ball->velocity = glm::vec3(8.0f, INITIAL_BALL_SPEED, 0.0f); createBricks(); }
void Game::createBricks() {
    glm::vec3 colors[] = { {0,0.5,1}, {0,1,0}, {1,1,0}, {1,0.5,0}, {1,0,0} };
    bricks.clear();
    for (int y = 0; y < 5; y++) for (int x = 0; x < 10; x++) bricks.push_back(new Brick(glm::vec3(-11.0f + x * 2.4f, 8.0f - y * 1.2f, 0.0f), glm::vec3(2.0f, 0.8f, 1.0f), colors[y]));
}
void Game::checkCollisions() {
    if (checkBallPaddleCollision()) {
        ball->reverseY();
        float hitPoint = (ball->position.x - paddle->position.x) / (paddle->size.x / 2.0f);
        ball->velocity.x = INITIAL_BALL_SPEED * hitPoint * 1.5f; 
        ball->position.y = paddle->position.y + (paddle->size.y / 2.0f) + ball->radius;
    }
    for (auto b : bricks) {
        if (!b->destroyed && checkBallBrickCollision(b)) { b->destroyed = true; score += 10; }
    }
}
bool Game::checkBallPaddleCollision() {
    bool colX = ball->position.x + ball->radius >= paddle->position.x - paddle->size.x/2 && paddle->position.x + paddle->size.x/2 >= ball->position.x - ball->radius;
    bool colY = ball->position.y - ball->radius <= paddle->position.y + paddle->size.y/2 && ball->position.y + ball->radius >= paddle->position.y - paddle->size.y/2;
    return colX && colY && ball->velocity.y < 0;
}
bool Game::checkBallBrickCollision(Brick* b) {
    glm::vec2 ballCenter(ball->position.x, ball->position.y);
    glm::vec2 halfExtents(b->size.x / 2.0f, b->size.y / 2.0f);
    glm::vec2 brickCenter(b->position.x, b->position.y);
    glm::vec2 diff = ballCenter - brickCenter;
    glm::vec2 clamped = glm::clamp(diff, -halfExtents, halfExtents);
    glm::vec2 closest = brickCenter + clamped;
    diff = closest - ballCenter;
    if (glm::length(diff) < ball->radius) {
        glm::vec2 compass[] = { {0,1}, {1,0}, {0,-1}, {-1,0} };
        float max = 0.0f; int best = -1;
        glm::vec2 n_diff = glm::normalize(-diff);
        for (int i=0; i<4; i++) { float dot = glm::dot(n_diff, compass[i]); if (dot > max) { max = dot; best = i; } }
        if (best == 1 || best == 3) { ball->reverseX(); float pen = ball->radius - std::abs(diff.x); ball->position.x += (best == 1) ? pen : -pen; }
        else { ball->reverseY(); float pen = ball->radius - std::abs(diff.y); ball->position.y += (best == 0) ? pen : -pen; }
        return true;
    }
    return false;
}