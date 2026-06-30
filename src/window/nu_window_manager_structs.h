#pragma once
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <tree/nu_node.h>

typedef struct NU_WindowDrawlist
{
    Array drawNodes;
    Array clippedDrawNodes;
    Array canvasNodes;
} NU_WindowDrawlist;

typedef struct NU_Window
{
    SDL_Window* window;
    NU_WindowDrawlist drawlist;
    NodeP* windowNode;
} NU_Window;

// Responsible for all window related functionality
typedef struct WindowManager
{
    Container windows;
    Array recycledSDLWindows;
    Array absoluteRootNodes;
    Hashmap clipMap;
    NodeP* hoveredWindowNode;
    SDL_Window* lastClickedWindow;
    int rootWindowID;
} WindowManager;
