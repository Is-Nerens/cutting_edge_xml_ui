#define _CRT_SECURE_NO_WARNINGS 

#include <SDL3/SDL.h>
#include "headers/parser.h"

void Calculate_Sizes(struct UI_Tree* ui_tree)
{
    // For each depth layer
    for (int l=MAX_TREE_DEPTH-1; l>0; l--)
    {
        // For each node in layer
        for (int i=0; i<ui_tree->tree_stack[l].size; i++)
        {
            struct Node* node = (struct Node*) Vector_Get(&ui_tree->tree_stack[l], i);
            node->width = 20;
            node->height = 20;
            node->x = 50;
            node->y = 50;
        }
    }
}

int ProcessWindowEvents()
{
    int isRunning = 1; 

    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        // CLOSE WINDOW EVENT
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)  {   
            isRunning = 0;
        }
        else if (event.type == SDL_EVENT_QUIT)               {
            isRunning = 0;
        }
        else if (event.type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
            // hovered_window_ID = event.window.windowID;
        }
        else if (event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
            // hovered_window_ID = 0;
        }
    }

    return isRunning;
}

int main()
{
    // Create an enpty UI tree stack
    struct UI_Tree ui_tree;

    // Parse xml into UI tree
    if (NU_Parse("tiny.xml", &ui_tree) != 0)
    {
        return -1;
    }

    // Check if SDL initialised
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("window", 500, 400, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, "direct3d");

    // Application loop
    int isRunning = 1;
    while (isRunning)
    {
        isRunning = ProcessWindowEvents();
        // Calculate element positions
        timer_start();
        start_measurement();
        Calculate_Sizes(&ui_tree);
        end_measurement();
        timer_stop();
        // GrowElements(root);
        // CalculatePositions(root, cursor);
    }

    // Free Memory
    NU_Free_UI_Tree_Memory(&ui_tree);
}