#include "application_ui.h"

using namespace std;

const int DEFAULT_SCREEN_WIDTH = 720, DEFAULT_SCREEN_HEIGHT = 480;

SDL_Window* init(string windowTitle, int width, int height)
{
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create window
        SDL_Window* gWindow = SDL_CreateWindow( 
            windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN
        );

        if( gWindow == NULL )
        {
            SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        
        return gWindow;
    }

    return NULL;
}

//Starts up SDL and creates window
SDL_Window* init(string windowTitle)
{
    init(windowTitle, DEFAULT_SCREEN_HEIGHT, DEFAULT_SCREEN_WIDTH);
}

SDL_Surface* loadMedia(string path)
{

    //Load splash image
    SDL_Surface* gHelloWorld = SDL_LoadBMP(path.c_str());
    if( gHelloWorld == NULL )
    {
        SDL_Log("Unable to load image ! SDL Error: %s\n", SDL_GetError());
    }

    return gHelloWorld;
}

void close(SDL_Window* gWindow, SDL_Renderer* renderer)
{
    //Deallocate surface
    SDL_DestroyRenderer(renderer);

    //Destroy window
    SDL_DestroyWindow( gWindow );

    //Quit SDL subsystems
    SDL_Quit();
}
