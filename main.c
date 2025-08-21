#define _CRT_SECURE_NO_WARNINGS 


#include <math.h>
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include "headers/parser.h"
#include "headers/layout.h"
#include "headers/resources.h"

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

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

    Vector_Reserve(&ui_tree.font_resources, sizeof(struct Font_Resource), 4);
    struct Font_Resource font1;
    struct Font_Resource font2;
    Load_Font_Resource("./Inter/Inter_Variable_Weight.ttf", &font1);
    Load_Font_Resource("./Inter/Inter_Variable_Weight.ttf", &font2);
    Vector_Push(&ui_tree.font_resources, &font1);
    Vector_Push(&ui_tree.font_resources, &font2);

    timer_start();
    start_measurement();

    // Parse xml into UI tree
    if (NU_Parse("test.xml", &ui_tree) != 0)
    {
        return -1;
    }

    end_measurement();
    timer_stop();

    // Check if SDL initialised
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }


    // Create a vector window, opengl_context and nano_vg_context pointers
    struct Vector windows;
    struct Vector gl_contexts;
    struct Vector nano_vg_contexts;
    Vector_Reserve(&windows, sizeof(SDL_Window*), 16);
    Vector_Reserve(&gl_contexts, sizeof(SDL_GLContext), 16);
    Vector_Reserve(&nano_vg_contexts, sizeof(NVGcontext*), 16);
    Vector_Reserve(&ui_tree.font_registries, sizeof(struct Vector), 16);

    struct NU_Watcher_Data watcher_data = {
        .ui_tree = &ui_tree,
        .windows = &windows,
        .gl_contexts = &gl_contexts,
        .nano_vg_contexts = &nano_vg_contexts
    };

    SDL_AddEventWatch(ResizingEventWatcher, &watcher_data);
    
    // Application loop
    int isRunning = 1;
    while (isRunning)
    {
        isRunning = ProcessWindowEvents();
        // Calculate element positions
        // timer_start();
        // start_measurement();
        NU_Clear_Node_Sizes(&ui_tree, &windows, &gl_contexts, &nano_vg_contexts);
        NU_Calculate_Text_Fit_Sizes(&ui_tree);
        NU_Calculate_Sizes(&ui_tree, &windows, &gl_contexts, &nano_vg_contexts);
        NU_Grow_Nodes(&ui_tree);
        NU_Calculate_Positions(&ui_tree);
        NU_Render(&ui_tree, &windows, &gl_contexts, &nano_vg_contexts);

        // end_measurement();
        // timer_stop();
    }

    // Free Memory
    NU_Free_UI_Tree_Memory(&ui_tree);
    Vector_Free(&windows);
    Vector_Free(&gl_contexts);
    Vector_Free(&nano_vg_contexts);

    // Close SDL
    SDL_Quit();
}












// #define _CRT_SECURE_NO_WARNINGS

// #include <stdio.h>
// #include <SDL3/SDL.h>
// #include <GL/glew.h>

// #define NANOVG_GL3_IMPLEMENTATION
// #include <nanovg.h>
// #include <nanovg_gl.h>

// int ProcessWindowEvents() {
//     int isRunning = 1;
//     SDL_Event event;
//     while (SDL_PollEvent(&event)) {
//         if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
//             isRunning = 0;
//         else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
//             isRunning = 0;
//     }
//     return isRunning;
// }

// int main(void) {
//     // Check if SDL initialised
//     if (!SDL_Init(SDL_INIT_VIDEO)) {
//         printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
//         return -1;
//     }

//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

//     SDL_Window* window = SDL_CreateWindow("NanoVG SDL3 Example", 800, 600, SDL_WINDOW_OPENGL);
//     if (!window) {
//         printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
//         SDL_Quit();
//         return -1;
//     }

//     SDL_GLContext glContext = SDL_GL_CreateContext(window);
//     if (!glContext) {
//         printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return -1;
//     }

//     glewExperimental = GL_TRUE;
//     if (glewInit() != GLEW_OK) {
//         printf("GLEW initialization failed!\n");
//         SDL_GL_DestroyContext(glContext);
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return -1;
//     }

//     NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
//     if (!vg) {
//         printf("NanoVG GL3 context creation failed!\n");
//         SDL_GL_DestroyContext(glContext);
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return -1;
//     }

//     int font = nvgCreateFont(vg, "inter", "./Inter/Inter_Variable_Weight.ttf");


//     int isRunning = 1;
//     while (isRunning) {
//         isRunning = ProcessWindowEvents();

//         int w, h;
//         SDL_GetWindowSize(window, &w, &h);
//         glViewport(0, 0, w, h);
//         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//         nvgBeginFrame(vg, w, h, 1.0f);
//         nvgFontFace(vg, "inter");
//         nvgFontSize(vg, 72.0f);
//         nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
//         nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
//         nvgText(vg, 0, 200, "The quick brown fox jumps over the lazy dog 1234567890", NULL);

//         nvgEndFrame(vg);

//         SDL_GL_SwapWindow(window);
//         SDL_Delay(16);
//     }

//     nvgDeleteGL3(vg);
//     SDL_GL_DestroyContext(glContext);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//     return 0;
// }
