#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>
#include <cmath>

#include "geometry.h"
#include <vector>
#include <glm/glm.hpp>
struct ObjectData
{
    GLuint vao; // this stores the vertex attributes pointer
    GLuint textureID;
    GLuint vertexBuffer;
    int vertexCount;
    glm::mat4 model;
    glm::vec4 colour;
    bool isEmissive;
    float angle = 0.0f;
    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
};

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render(int timeElapsed);
    bool handleEvent(SDL_Event e);
    void cleanup();
    void setCameraSpeed(float speed);

private:
    SDL_Window* sdlWin;

    // GLuint vao;
    GLuint shader;
    float totalTime = 0.0f;
    std::vector<ObjectData> objects;

    int start = 1;
    float alpha = 1.0f;
    float beta = 1.0f;


    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
    float cameraSpeed = 0.8f;
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

#endif
