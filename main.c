// #define _CRT_SECURE_NO_WARNINGS 


// #include <math.h>
// #include <SDL3/SDL.h>
// #include <GL/glew.h>
// #include "headers/parser.h"
// #include "headers/layout.h"
// #include "headers/resources.h"

// #define NANOVG_GL3_IMPLEMENTATION
// #include <nanovg.h>
// #include <nanovg_gl.h>
// #include <cairo.h>
// #include <freetype/freetype.h>

// int ProcessWindowEvents()
// {
//     int isRunning = 1; 

//     SDL_Event event;
//     while (SDL_PollEvent(&event)) 
//     {
//         // CLOSE WINDOW EVENT
//         if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)  {   
//             isRunning = 0;
//         }
//         else if (event.type == SDL_EVENT_QUIT)               {
//             isRunning = 0;
//         }
//         else if (event.type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
//             // hovered_window_ID = event.window.windowID;
//         }
//         else if (event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
//             // hovered_window_ID = 0;
//         }
//     }

//     return isRunning;
// }

// int main()
// {
//     // Create an enpty UI tree stack
//     struct UI_Tree ui_tree;

//     Vector_Reserve(&ui_tree.font_resources, sizeof(struct Font_Resource), 4);
//     struct Font_Resource font1;
//     struct Font_Resource font2;
//     Load_Font_Resource("./Inter/Inter_Variable_Weight.ttf", &font1);
//     Load_Font_Resource("./Inter/Inter_Variable_Weight.ttf", &font2);
//     Vector_Push(&ui_tree.font_resources, &font1);
//     Vector_Push(&ui_tree.font_resources, &font2);

//     timer_start();
//     start_measurement();

//     // Parse xml into UI tree
//     if (NU_Parse("test.xml", &ui_tree) != 0)
//     {
//         return -1;
//     }

//     end_measurement();
//     timer_stop();

//     // Check if SDL initialised
//     if (!SDL_Init(SDL_INIT_VIDEO)) {
//         printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
//         return -1;
//     }


//     // Create a vector window, opengl_context and nano_vg_context pointers
//     struct Vector windows;
//     struct Vector gl_contexts;
//     struct Vector nano_vg_contexts;
//     Vector_Reserve(&windows, sizeof(SDL_Window*), 16);
//     Vector_Reserve(&gl_contexts, sizeof(SDL_GLContext), 16);
//     Vector_Reserve(&nano_vg_contexts, sizeof(NVGcontext*), 16);
//     Vector_Reserve(&ui_tree.font_registries, sizeof(struct Vector), 16);

//     struct NU_Watcher_Data watcher_data = {
//         .ui_tree = &ui_tree,
//         .windows = &windows,
//         .gl_contexts = &gl_contexts,
//         .nano_vg_contexts = &nano_vg_contexts
//     };

//     SDL_AddEventWatch(ResizingEventWatcher, &watcher_data);
    
//     // Application loop
//     int isRunning = 1;
//     while (isRunning)
//     {
//         isRunning = ProcessWindowEvents();
//         // Calculate element positions
//         // timer_start();
//         // start_measurement();
//         NU_Clear_Node_Sizes(&ui_tree, &windows, &gl_contexts, &nano_vg_contexts);
//         NU_Calculate_Text_Fit_Sizes(&ui_tree);
//         NU_Calculate_Sizes(&ui_tree, &windows, &gl_contexts, &nano_vg_contexts);
//         NU_Grow_Nodes(&ui_tree);
//         NU_Calculate_Positions(&ui_tree);
//         NU_Render(&ui_tree, &windows, &gl_contexts, &nano_vg_contexts);

//         // end_measurement();
//         // timer_stop();
//     }

//     // Free Memory
//     NU_Free_UI_Tree_Memory(&ui_tree);
//     Vector_Free(&windows);
//     Vector_Free(&gl_contexts);
//     Vector_Free(&nano_vg_contexts);

//     // Close SDL
//     SDL_Quit();
// }








#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main() 
{
    // ------------------ SDL3 + OpenGL ------------------
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Text Rendering", 700, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    glewInit();

    int display_w, display_h;
    SDL_GetWindowSize(window, &display_w, &display_h);

    SDL_DisplayID display_id = SDL_GetDisplayForWindow(window);
    float hidpi_scale = 1.0f;
    if (display_id) {
        hidpi_scale = SDL_GetDisplayContentScale(display_id);
    }

    // ------------------ FreeType ------------------
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        printf("Failed to init FreeType\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "fonts/Inter/Inter_Variable_Weight.ttf", 0, &face)) {
        printf("Failed to load font. Trying fallback...\n");
        if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face)) {
            printf("Failed fallback font\n");
            FT_Done_FreeType(ft);
            return 1;
        }
    }

    float dpi = 96.0f * hidpi_scale;
    float font_size = 18.0f;
    unsigned int font_pixels_v = (unsigned int)roundf((dpi / 72.0f) * font_size);

    FT_Set_Pixel_Sizes(face, 0, font_pixels_v);

    // ------------------ Cairo ------------------
    unsigned char* pixels = malloc(display_w * display_h * 4);
    if (!pixels) return 1;

    cairo_surface_t* surface = cairo_image_surface_create_for_data(
        pixels, CAIRO_FORMAT_ARGB32, display_w, display_h, display_w * 4
    );
    cairo_t* cr = cairo_create(surface);

    if (hidpi_scale > 1.0f) cairo_scale(cr, 1.0 / hidpi_scale, 1.0 / hidpi_scale);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_font_face_t* cairo_face = cairo_ft_font_face_create_for_ft_face(face, 0);

    cairo_font_options_t* opts = cairo_font_options_create();
    cairo_font_options_set_antialias(opts, CAIRO_ANTIALIAS_SUBPIXEL);
    cairo_font_options_set_hint_style(opts, CAIRO_HINT_STYLE_FULL);
    cairo_font_options_set_hint_metrics(opts, CAIRO_HINT_METRICS_ON);
    cairo_set_font_options(cr, opts);
    cairo_font_options_destroy(opts);

    cairo_set_font_face(cr, cairo_face);
    cairo_set_font_size(cr, (double)font_pixels_v);

    cairo_set_source_rgb(cr, 0.1, 1.0, 0.1);
    cairo_move_to(cr, 50.5, 100.5);
    cairo_show_text(cr, "Crystal Clear Text!");
    cairo_move_to(cr, 50.5, 150.5);
    cairo_show_text(cr, "SDL3 + FreeType + Cairo");

    // ------------------ OpenGL Texture ------------------
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, display_w, display_h, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

    // ------------------ Render loop ------------------
    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = 0;
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(-1, -1);
            glTexCoord2f(1, 1); glVertex2f(1, -1);
            glTexCoord2f(1, 0); glVertex2f(1, 1);
            glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glEnd();

        glDisable(GL_BLEND);

        SDL_GL_SwapWindow(window);
    }

    // ------------------ Cleanup ------------------
    glDeleteTextures(1, &tex);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    cairo_font_face_destroy(cairo_face);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(pixels);
    return 0;
}
