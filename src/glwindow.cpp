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

// Advances obj.angle by orbitSpeed * multiplier * dt, then returns the
// orbital frame: parent rotated by the new angle, translated out by orbitRadius.
static glm::mat4 orbitFrame(const glm::mat4& parent, ObjectData& obj, float multiplier, float dt)
{
    obj.angle += obj.orbitSpeed * multiplier * dt;
    glm::mat4 frame = glm::rotate(parent, obj.angle, glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::translate(frame, glm::vec3(obj.orbitRadius, 0.0f, 0.0f));
}

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

ObjectData createObject(const GeometryData& geometry, const GLuint& shader, const glm::mat4& modelMatrix, const glm::vec4& colour=glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), const char* texturePath=nullptr, bool isEmissive=false){
    ObjectData obj;
    obj.isEmissive = isEmissive;


    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

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
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            obj.textureID = texture;
        }
        stbi_image_free(data);
    }

    // build interleaved buffer: [pos.xyz, normal.xyz, texcoord.uv] = 8 floats/vertex
    const int count = geometry.vertexCount();
    const float* verts = (const float*)geometry.vertexData();
    const float* norms = (const float*)geometry.normalData();
    const float* texs  = (const float*)geometry.textureCoordData();

    std::vector<float> interleaved;
    interleaved.reserve(count * 8);
    for (int i = 0; i < count; i++) {
        interleaved.push_back(verts[i*3+0]);
        interleaved.push_back(verts[i*3+1]);
        interleaved.push_back(verts[i*3+2]);
        interleaved.push_back(norms[i*3+0]);
        interleaved.push_back(norms[i*3+1]);
        interleaved.push_back(norms[i*3+2]);
        interleaved.push_back(texs[i*2+0]);
        interleaved.push_back(texs[i*2+1]);
    }

    GLuint vertexBuffer = 0;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, count * 8 * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    const GLsizei stride = 8 * sizeof(float);
    int posLoc = glGetAttribLocation(shader, "position");
    // the vbo starts from position, then normal, and then TexCoord
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, false, stride, (void*)0);
    glEnableVertexAttribArray(posLoc);

    int normLoc = glGetAttribLocation(shader, "normal");
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(normLoc);

    int texLoc = glGetAttribLocation(shader, "TexCoord");
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(texLoc);

    obj.vao = vao;
    obj.vertexCount = count;
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



    // [0] Sun
    ObjectData sun = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(255.0f, 200.0f, 0.0f, 255.0f), "../res/sun_diffuse0.jpg", true);

    // [1] Mercury
    ObjectData mercury = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(180.0f, 180.0f, 180.0f, 255.0f), "../res/mercury.jpg");
    mercury.angle = 0.0f;
    mercury.orbitRadius = 4.0f;
    mercury.orbitSpeed = 2.0f;

    // [2] Venus
    ObjectData venus = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(220.0f, 180.0f, 100.0f, 255.0f), "../res/venus.jpg");
    venus.angle = M_PI / 3.0f;
    venus.orbitRadius = 7.0f;
    venus.orbitSpeed = 1.3f;

    // [3] Earth
    ObjectData earth = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(0.0f, 255.0f, 0.0f, 255.0f), "../res/earth_diffuse.png");
    earth.angle = M_PI;
    earth.orbitRadius = 10.0f;
    earth.orbitSpeed = 0.8f;

    // [4] Moon
    ObjectData moon = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(200.0f, 200.0f, 200.0f, 255.0f), "../res/moon_diffuse.png");
    moon.angle = 3.0f * M_PI / 2.0f;
    moon.orbitRadius = 1.5f;
    moon.orbitSpeed = 3.0f;

    // [5] Mars
    ObjectData mars = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(200.0f, 80.0f, 50.0f, 255.0f), "../res/mars.jpg");
    mars.angle = M_PI / 2.0f;
    mars.orbitRadius = 13.0f;
    mars.orbitSpeed = 0.5f;

    // [6] Jupiter
    ObjectData jupiter = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(200.0f, 150.0f, 100.0f, 255.0f), "../res/jupyter.jpg");
    jupiter.angle = M_PI;
    jupiter.orbitRadius = 17.0f;
    jupiter.orbitSpeed = 0.25f;

    // [7] Saturn
    ObjectData saturn = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(210.0f, 180.0f, 130.0f, 255.0f), "../res/saturn.jpg");
    saturn.angle = 3.0f * M_PI / 4.0f;
    saturn.orbitRadius = 21.0f;
    saturn.orbitSpeed = 0.15f;

    // [8] Uranus
    ObjectData uranus = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(150.0f, 220.0f, 230.0f, 255.0f), "../res/uranus.jpg");
    uranus.angle = M_PI / 4.0f;
    uranus.orbitRadius = 25.0f;
    uranus.orbitSpeed = 0.1f;

    // [9] Neptune
    ObjectData neptune = createObject(geometry, shader, glm::mat4(1.0f), glm::vec4(60.0f, 80.0f, 200.0f, 255.0f), "../res/neptune.jpg");
    neptune.angle = 5.0f * M_PI / 4.0f;
    neptune.orbitRadius = 29.0f;
    neptune.orbitSpeed = 0.07f;

    objects.push_back(sun);      // [0]
    objects.push_back(mercury);  // [1]
    objects.push_back(venus);    // [2]
    objects.push_back(earth);    // [3]
    objects.push_back(moon);     // [4]
    objects.push_back(mars);     // [5]
    objects.push_back(jupiter);  // [6]
    objects.push_back(saturn);   // [7]
    objects.push_back(uranus);   // [8]
    objects.push_back(neptune);  // [9]


    // set up camera
    view = glm::lookAt(cameraPos, glm::vec3(0.0f), cameraUp);

    // GLuint eyeDirectionLoc = glGetUniformLocation(shader, "eyeDirection");
    // glUniform3fv(eyeDirectionLoc, 1, glm::value_ptr(-cameraPos));
    // glm::mat4 view = glm::mat4(1.0f);
    // view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f)); // move camera backwards
    
    
    glm::mat4 projection = glm::mat4(1.0f);
    // projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    projection = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, -50.0f, 50.0f);
    
    GLuint viewLoc = 0;
    GLuint projectionLoc = 0;
    viewLoc = glGetUniformLocation(shader, "view");
    projectionLoc = glGetUniformLocation(shader, "projection");

    // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // set up lighting
    glm::vec3 lightColor = glm::vec3(1.0, 0.95, 0.8);
    glUniform3fv(glGetUniformLocation(shader, "lightColour"), 1, glm::value_ptr(lightColor));

    // the light position is always in the centre
    glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

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

    GLuint eyeDirectionLoc = glGetUniformLocation(shader, "eyeDirection");
    glUniform3fv(eyeDirectionLoc, 1, glm::value_ptr(-rotatedEye));

    glm::mat4 sun_frame = glm::mat4(1.0f);
    objects[0].model = glm::scale(sun_frame, glm::vec3(3.0f, 3.0f, 3.0f));

    glm::mat4 mercury_frame = orbitFrame(sun_frame, objects[1], alpha, dt);
    objects[1].model = glm::scale(mercury_frame, glm::vec3(0.4f, 0.4f, 0.4f));

    glm::mat4 venus_frame = orbitFrame(sun_frame, objects[2], alpha, dt);
    objects[2].model = glm::scale(venus_frame, glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 earth_frame = orbitFrame(sun_frame, objects[3], alpha, dt);
    objects[3].model = glm::scale(earth_frame, glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 moon_frame = orbitFrame(earth_frame, objects[4], beta, dt);
    objects[4].model = glm::scale(moon_frame, glm::vec3(0.15f, 0.15f, 0.15f));

    glm::mat4 mars_frame = orbitFrame(sun_frame, objects[5], alpha, dt);
    objects[5].model = glm::scale(mars_frame, glm::vec3(0.9f, 0.9f, 0.9f));

    glm::mat4 jupiter_frame = orbitFrame(sun_frame, objects[6], alpha, dt);
    objects[6].model = glm::scale(jupiter_frame, glm::vec3(1.8f, 1.8f, 1.8f));

    glm::mat4 saturn_frame = orbitFrame(sun_frame, objects[7], alpha, dt);
    objects[7].model = glm::scale(saturn_frame, glm::vec3(1.5f, 1.5f, 1.5f));

    glm::mat4 uranus_frame = orbitFrame(sun_frame, objects[8], alpha, dt);
    objects[8].model = glm::scale(uranus_frame, glm::vec3(1.2f, 1.2f, 1.2f));

    glm::mat4 neptune_frame = orbitFrame(sun_frame, objects[9], alpha, dt);
    objects[9].model = glm::scale(neptune_frame, glm::vec3(1.2f, 1.2f, 1.2f));


    for (const ObjectData& obj : objects) {
        GLuint modelLoc = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
        GLuint colourLoc = glGetUniformLocation(shader, "colour");
        glUniform4fv(colourLoc, 1, glm::value_ptr(obj.colour));
        // is the object an light source?
        glUniform1i(glGetUniformLocation(shader, "isEmissive"), obj.isEmissive);

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
