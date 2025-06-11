#define _CRT_SECURE_NO_WARNINGS 

#include <math.h>
#include <SDL3/SDL.h>
#include "headers/parser.h"

void Calculate_Sizes(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* renderers)
{
    // Compute sizes of nodes in the deepest layer
    if (ui_tree->deepest_layer > 0)
    {
        struct Vector* layer = &ui_tree->tree_stack[ui_tree->deepest_layer];

        // For each node in the deepest layer
        for (int i=0; i<layer->size; i++)
        {
            struct Node* node = Vector_Get(layer, i);
            node->width = 1;
            node->height = 1;
            node->x = 1;
            node->y = 1;
        }
    }

    // For each layer exluding the deepest
    for (int l=ui_tree->deepest_layer-1; l>0; l--)
    {
        struct Vector* parent_layer = &ui_tree->tree_stack[l];
        struct Vector* child_layer = &ui_tree->tree_stack[l+1];
        
        // Iterate over layer
        for (int p=0; p<parent_layer->size; p++)
        {   
            struct Node* parent = Vector_Get(parent_layer, p);
            int is_layout_horizontal = (parent->layout_flags & 0x01) == LAYOUT_HORIZONTAL;

            // Track the total width and height for the parent's content
            float contentWidth = 0;
            float contentHeight = 0;

            // Iterate over children
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
            {
                struct Node* child = Vector_Get(child_layer, i);

                if (child->tag == WINDOW)
                {
                    if (is_layout_horizontal) contentWidth -= parent->gap;
                    if (!is_layout_horizontal) contentHeight -= parent->gap;
                }

                // Horizontal Layout
                if (is_layout_horizontal)
                {
                    contentWidth += child->width;
                    contentHeight = MAX(contentHeight, child->height);
                }

                // Vertical Layout
                if (!is_layout_horizontal)
                {
                    contentHeight += child->height;
                    contentWidth = MAX(contentWidth, child->width);
                }

                // If child is window node and has no SDL window assigned to it
                if (child->tag == WINDOW && child->window == NULL)
                {
                    // Create a new window and renderer
                    SDL_Window* new_window = SDL_CreateWindow("window", 500, 400, SDL_WINDOW_RESIZABLE);
                    SDL_Renderer* new_renderer = SDL_CreateRenderer(new_window, "direct3d");
                    Vector_Push(windows, new_window);
                    Vector_Push(renderers, new_renderer);
                    child->window = new_window;
                    child->renderer = new_renderer;
                }
            }

            if (parent->child_count > 0 && parent->tag != WINDOW)
            {
                // Horizontal Layout
                if (is_layout_horizontal) contentWidth += (parent->child_count) * parent->gap;

                // Vertical layout
                if (!is_layout_horizontal) contentHeight += (parent->child_count) * parent->gap;

                parent->width = contentWidth + parent->border_left + parent->border_right + parent->pad_left + parent->pad_right;
                parent->height = contentHeight + parent->border_top + parent->border_bottom + parent->pad_top + parent->pad_bottom;
            }
        }
    }
}

void Grow_Child_Nodes_H(struct Node* parent, struct Vector* child_layer)
{
    int is_layout_horizontal = (parent->layout_flags & 0x01) == LAYOUT_HORIZONTAL;
    float remaining_width = parent->width;

    if (!is_layout_horizontal)
    {   
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->growth == 1 || child->growth == 3)
            {
                float rem_width = remaining_width - child->width;
                child->width += rem_width; 
            }
        }
    }
    else
    {
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->tag == WINDOW) remaining_width += parent->gap;
            else remaining_width -= child->width;
        }

        remaining_width -= (parent->child_count - 1) * parent->gap;

        if (remaining_width <= 0) return;

        // Count growable elements
        uint32_t growable_count = 0;
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->growth == 1 || child->growth == 3 && child->tag != WINDOW) growable_count += 1;
        }

        if (growable_count == 0) return;

        float growth_per_element = remaining_width / (float) growable_count;

        // Grow child elements
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->growth == 1 || child->growth == 3 && child->tag != WINDOW)
            {
                child->width += growth_per_element;
                child->width = roundf(child->width);
            }
        }
    }
}

void Grow_Child_Nodes_V(struct Node* parent, struct Vector* child_layer)
{
    int is_layout_horizontal = (parent->layout_flags & 0x01) == LAYOUT_HORIZONTAL;
    float remaining_height = parent->height;

    if (is_layout_horizontal)
    {
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->growth == 2 || child->growth == 3)
            {
                float rem_height = remaining_height - child->height;
                child->height += rem_height; 
            }
        }
    }
    else
    {
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->tag == WINDOW) remaining_height += parent->gap;
            else remaining_height -= child->height;
        }

        remaining_height-= (parent->child_count - 1) * parent->gap;

        if (remaining_height <= 0) return;

        // Count growable elements
        uint32_t growable_count = 0;
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->growth == 2 || child->growth == 3 && child->tag != WINDOW) growable_count += 1;
        }

        if (growable_count == 0) return;

        float growth_per_element = remaining_height / (float) growable_count;

        // Grow child elements
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->growth == 2 || child->growth == 3 && child->tag != WINDOW)
            {
                child->height += growth_per_element;
                child->height = roundf(child->height);
            }
        }
    }
}

void Grow_Nodes(struct UI_Tree* ui_tree)
{
    // For each layer
    for (int l=0; l<ui_tree->deepest_layer; l++)
    {
        struct Vector* parent_layer = &ui_tree->tree_stack[l];
        struct Vector* child_layer = &ui_tree->tree_stack[l+1];
        
        // For node in layer
        for (int p=0; p<parent_layer->size; p++)
        {   
            struct Node* parent = Vector_Get(parent_layer, p);

            Grow_Child_Nodes_H(parent, child_layer);
            Grow_Child_Nodes_V(parent, child_layer);
        }
    }
}

void Calculate_Positions(struct UI_Tree* ui_tree)
{

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

    // Create a window and renderer
    SDL_Window* window = SDL_CreateWindow("window", 500, 400, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, "direct3d");

    // Create a vector of window pointers and store main window
    struct Vector windows;
    Vector_Reserve(&windows, sizeof(SDL_Window*), 8);
    Vector_Push(&windows, window);

    // Create a vector of renderer pointers and store main renderer
    struct Vector renderers;
    Vector_Reserve(&renderers, sizeof(SDL_Renderer*), 8);
    Vector_Push(&renderers, renderer);

    printf("%s %zu\n", "node bytes:", sizeof(struct Node));

    // Application loop
    int isRunning = 1;
    while (isRunning)
    {
        isRunning = ProcessWindowEvents();
        // Calculate element positions
        timer_start();
        start_measurement();
        Calculate_Sizes(&ui_tree, &windows, &renderers);
        Grow_Nodes(&ui_tree);
        Calculate_Positions(&ui_tree);
        end_measurement();
        timer_stop();
    }

    // Free Memory
    NU_Free_UI_Tree_Memory(&ui_tree);
    Vector_Free(&windows);
    Vector_Free(&renderers);
}