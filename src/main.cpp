#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "glwindow.h"

int main(int argc, char* argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to initialize SDL", 0);
        return 1;
    }

    OpenGLWindow window;
    window.initGL();

    bool running = true;
    int now = SDL_GetTicks();

    // for camera update
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while(running)
    {   
        // Calculate delta time
        float currentFrame = SDL_GetTicks() / 1000.0f; // sdl returns ms, convert to seconds
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        window.setCameraSpeed(10.0f * deltaTime);

        // Check for a quit event before passing to the GLWindow
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {   
            if(e.type == SDL_QUIT)
            {
                running = false;
            }
            else if(!window.handleEvent(e))
            {
                running = false;
            }
        }
        int currentTime = SDL_GetTicks();
        int timeElapsed = currentTime - now;
        
        window.render(timeElapsed);

        now = currentTime;
        // We sleep for 10ms here so as to prevent excessive CPU usage
        SDL_Delay(10);
    }

    window.cleanup();
    SDL_Quit();
    return 0;
}

