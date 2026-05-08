#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>

#include "geometry.h"
#include <vector>
#include <glm/glm.hpp>
struct ObjectData
{
    GLuint vao; // this stores the vertex attributes pointer
    GLuint textureID;
    // Used only for cleanup
    GLuint textureBuffer;
    GLuint vertexBuffer;
    int vertexCount;
    glm::mat4 model;
};

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    bool handleEvent(SDL_Event e);
    void cleanup();

private:
    SDL_Window* sdlWin;

    // GLuint vao;
    GLuint shader;
    // GLuint vertexBuffer;
    std::vector<ObjectData> objects;
};

#endif
