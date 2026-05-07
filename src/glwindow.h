#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>

#include "geometry.h"
#include <vector>

struct ObjectData
{
    GLuint vertexBuffer; // this stores the vertex data
    GLuint vao; // this stores the vertex attributes pointer
    int vertexCount;
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
