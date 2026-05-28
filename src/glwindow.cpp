#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

using namespace std;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}


GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

glm::vec4 rgbMap(glm::vec4 color) {
    color = color / 255.0f;
    return color;
}

ObjectData createObject(const GeometryData& geometry, const GLuint& shader, const glm::mat4& modelMatrix, const glm::vec4& colour=glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), const char* texturePath=nullptr){
    // draw a single object and return the object data
    ObjectData obj;

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Texture
    if (texturePath != nullptr) {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

        if (data){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            obj.textureID = texture;
            int texLoc = glGetAttribLocation(shader, "TexCoord");
            GLuint texBuffer = 0;
            glGenBuffers(1, &texBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, texBuffer);
            glBufferData(GL_ARRAY_BUFFER, geometry.vertexCount() * 2 * sizeof(float), geometry.textureCoordData(), GL_STATIC_DRAW);
            glVertexAttribPointer(texLoc, 2, GL_FLOAT, false, 0, 0);
            glEnableVertexAttribArray(texLoc);
        }

        stbi_image_free(data);
    }


    int vertexLoc = glGetAttribLocation(shader, "position");
    GLuint vertexBuffer = 0;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, geometry.vertexCount() * 3 * sizeof(float), geometry.vertexData(), GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    obj.vao = vao;
    obj.vertexCount = geometry.vertexCount();
    obj.colour = rgbMap(colour);
    obj.vertexBuffer = vertexBuffer;
    return obj;
}

void drawObject(const ObjectData& obj){
    glBindVertexArray(obj.vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj.vertexBuffer);
    glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
}

OpenGLWindow::OpenGLWindow()
{
}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 640, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);
    // Load the model that we want to use and buffer the vertex attributes
    GeometryData geometry;
    geometry.loadFromOBJFile("../res/sphere-fixed.txt");



    ObjectData sun = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(255.0f, 0.0f, 0.0f, 255.0f), "../res/sun_diffuse0.jpg");
    ObjectData earth = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(0.0f, 255.0f, 0.0f, 255.0f), "../res/earth_diffuse.png");
    ObjectData moon = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(0.0f, 0.0f, 255.0f, 255.0f), "../res/moon_diffuse.png");
    objects.push_back(sun);
    objects.push_back(earth);
    objects.push_back(moon);


    // set up camera
    view = glm::lookAt(cameraPos, glm::vec3(0.0f), cameraUp);
    // glm::mat4 view = glm::mat4(1.0f);
    // view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f)); // move camera backwards
    
    
    glm::mat4 projection = glm::mat4(1.0f);
    // projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    projection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -30.0f, 30.0f);
    
    GLuint viewLoc = 0;
    GLuint projectionLoc = 0;
    viewLoc = glGetUniformLocation(shader, "view");
    projectionLoc = glGetUniformLocation(shader, "projection");

    // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glPrintError("Setup complete", true);
}



void OpenGLWindow::render(int timeElapsed)
{   
    float dt;
    if (start == 1) {
        dt = timeElapsed / 1000.0f;
    }
    else {
        dt = 0.0f;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint viewLoc = glGetUniformLocation(shader, "view");
    
    glm::mat4 cameraR = glm::mat4(1.0f);
    cameraR = glm::rotate(cameraR, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    cameraR = glm::rotate(cameraR, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    cameraR = glm::rotate(cameraR, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 rotatedEye = cameraR * glm::vec4(cameraPos, 1.0f);
    glm::vec3 rotatedUp = cameraR * glm::vec4(cameraUp, 1.0f);

    view = glm::lookAt(rotatedEye, glm::vec3(0.0f), rotatedUp);


    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Sun stationary
    glm::mat4 sun_pivot = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    objects[0].model = glm::scale(sun_pivot, glm::vec3(2.0f, 2.0f, 2.0f));

    // Earth accumulate angle so speed changes don't cause jumps
    earthAngle += 0.5f * alpha * dt ;
    float earth_radius = 5.0f;
    glm::mat4 earth_pivot = glm::translate(sun_pivot, glm::vec3(earth_radius * cos(earthAngle), earth_radius * sin(earthAngle), 0.0f));
    objects[1].model = glm::scale(earth_pivot, glm::vec3(0.7f, 0.7f, 0.7f));

    // Moon accumulate angle, orbit relative to unscaled earth_pivot
    moonAngle += 1.0f * beta * dt;
    float moon_radius = 1.2f;
    glm::mat4 moon_pivot = glm::translate(earth_pivot, glm::vec3(moon_radius * cos(moonAngle), moon_radius * sin(moonAngle), 0.0f));
    objects[2].model = glm::scale(moon_pivot, glm::vec3(0.3f, 0.3f, 0.3f));



    for (const ObjectData& obj : objects) {
        GLuint modelLoc = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
        GLuint colourLoc = glGetUniformLocation(shader, "colour");
        glUniform4fv(colourLoc, 1, glm::value_ptr(obj.colour));

        GLuint textureLoc = glGetUniformLocation(shader, "TexCoord");
        glActiveTexture(GL_TEXTURE0);                          // activate unit 0
        glBindTexture(GL_TEXTURE_2D, obj.textureID);           // bind texture to unit 0
        glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0);

        glBindVertexArray(obj.vao);
        glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
    }
    
    // wap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {   

        
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
        else if(e.key.keysym.sym == SDLK_SPACE)
        {
            if (start == 0) {
                start = 1;
            } else {
                start = 0;
            }
        }
        else if(e.key.keysym.sym == SDLK_UP)
        {
            alpha += 0.1f;
        }
        else if(e.key.keysym.sym == SDLK_DOWN)
        {
            alpha -= 0.1f;
        }
        else if(e.key.keysym.sym == SDLK_LEFT)
        {
            beta += 0.1f;
        }
        else if(e.key.keysym.sym == SDLK_RIGHT)
        {
            beta -= 0.1f;
        }

       
        if (e.key.keysym.sym == SDLK_a){
            // cameraPos += yaw * cameraSpeed;
            yaw += cameraSpeed;
        }
        else if (e.key.keysym.sym == SDLK_d){
            // cameraPos -= yaw * cameraSpeed;
            yaw -= cameraSpeed;
        }
        else if (e.key.keysym.sym == SDLK_w){
            // cameraPos -= glm::normalize(glm::cross(cameraUp, cameraForward)) * cameraSpeed;
            pitch += cameraSpeed;
        }
        else if (e.key.keysym.sym == SDLK_s){
            // cameraPos -= pitch * cameraSpeed;
            pitch -= cameraSpeed;
        }

        else if (e.key.keysym.sym == SDLK_q){
            // cameraPos += cameraUp * cameraSpeed;
            roll += cameraSpeed;
        }
        else if (e.key.keysym.sym == SDLK_e){
            // cameraPos -= cameraUp * cameraSpeed;
            roll -= cameraSpeed;
        }
    }
    return true;
}

void OpenGLWindow::cleanup()
{   
    for (ObjectData& obj : objects) {  
        glDeleteVertexArrays(1, &obj.vao);
        glDeleteBuffers(1, &obj.vertexBuffer);
    }
    SDL_DestroyWindow(sdlWin);
}

void OpenGLWindow::setCameraSpeed(float speed)
{
    cameraSpeed = speed;
}
