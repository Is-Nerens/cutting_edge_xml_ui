#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <math.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

// UI layout ------------------------------------------------------------
struct NU_Cursor
{
    float x, y, gap;
    char is_vertical;
};

void NU_Create_New_Window(struct UI_Tree* ui_tree, struct Node* window_node, struct Vector* windows, struct Vector* gl_contexts, struct Vector* nano_vg_contexts)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* new_window = SDL_CreateWindow("window", 500, 400, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext new_gl_context = SDL_GL_CreateContext(new_window);
    SDL_GL_MakeCurrent(new_window, new_gl_context);
    glewInit();
    

    NVGcontext* new_nano_vg_context = nvgCreateGL3(NVG_STENCIL_STROKES);
    struct Vector font_registry;
    Vector_Reserve(&font_registry, sizeof(int), 8);
    for (int i=0; i<ui_tree->font_resources.size; i++)
    {
        struct Font_Resource* font = Vector_Get(&ui_tree->font_resources, i);
        int fontID = nvgCreateFontMem(new_nano_vg_context, font->name, font->data, font->size, 0);
        Vector_Push(&font_registry, &fontID);
    }
    Vector_Push(windows, &new_window);
    Vector_Push(gl_contexts, &new_gl_context);
    Vector_Push(nano_vg_contexts, &new_nano_vg_context);
    Vector_Push(&ui_tree->font_registries, &font_registry);
    window_node->window = new_window;
    window_node->vg = new_nano_vg_context;
}

void NU_Reset_Node_size(struct Node* node)
{
    node->border_left = 1;
    node->border_right = 1;
    node->border_top = 1;
    node->border_bottom = 1;
    node->pad_left = 0;
    node->pad_right = 0;
    node->pad_top = 0;
    node->pad_bottom = 0;
    if (node->preferred_width == 0.0f) node->width = node->border_left + node->border_right + node->pad_left + node->pad_right;
    else node->width = node->preferred_width;
    node->height = node->border_top + node->border_bottom + node->pad_top + node->pad_bottom;
    node->gap = 0.0f;
}

void NU_Clear_Node_Sizes(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* gl_contexts, struct Vector* nano_vg_contexts)
{
    // For each layer
    for (int l=0; l<=ui_tree->deepest_layer; l++)
    {
        struct Vector* parent_layer = &ui_tree->tree_stack[l];
        struct Vector* child_layer = &ui_tree->tree_stack[l+1];

        for (int p=0; p<parent_layer->size; p++)
        {       
            // Iterate over layer
            struct Node* parent = Vector_Get(parent_layer, p);

            // If parent is window node and has no SDL window assigned to it
            if (parent->tag == WINDOW && parent->window == NULL)
            {
                // Create a new window and renderer
                NU_Create_New_Window(ui_tree, parent, windows, gl_contexts, nano_vg_contexts);
            }

            if (parent->child_count == 0) continue; // Skip acummulating child sizes (no children)

            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
            {
                struct Node* child = Vector_Get(child_layer, i);

                // Inherit window and renderer from parent
                if (child->tag != WINDOW && child->window == NULL)
                {
                    child->window = parent->window;
                    child->vg = parent->vg;
                }

                NU_Reset_Node_size(child);
            }
        }
    }
}


void NU_Calculate_Text_Fit_Size(struct UI_Tree* ui_tree, struct Node* node, struct Text_Ref* text_ref)
{
    // Extract pointer to text
    char* text = ui_tree->text_arena.char_buffer.data + text_ref->buffer_index;
    float bounds[4];  // left, top, right, bottom
    float width, height;

    // Make sure the NanoVG context has the correct font/size set before measuring!
    struct Vector* font_registry = Vector_Get(&ui_tree->font_registries, 0);
    int fontID = *(int*) Vector_Get(font_registry, 0);
    nvgFontFaceId(node->vg, fontID);   
    nvgFontSize(node->vg, 18);

    // Calculate text bounds
    nvgTextBounds(node->vg, 0, 0, text, text + text_ref->char_count, bounds);
    width  = bounds[2] - bounds[0];
    height = bounds[3] - bounds[1];


    // Apply size (only set preferred width if not specified)
    if (node->preferred_width == 0.0f)
        node->width += width;
    node->height += height;
}

void NU_Calculate_Text_Fit_Sizes(struct UI_Tree* ui_tree)
{
    for (uint32_t i=0; i<ui_tree->text_arena.text_refs.size; i++)
    {
        struct Text_Ref* text_ref = (struct Text_Ref*) Vector_Get(&ui_tree->text_arena.text_refs, i);
        
        // Get the corresponding node
        uint8_t node_depth = (uint8_t)(text_ref->node_ID >> 24);
        uint32_t node_index = text_ref->node_ID & 0x00FFFFFF;
        struct Vector* layer = &ui_tree->tree_stack[node_depth];
        struct Node* node = Vector_Get(layer, node_index);

        // Calculate text size
        NU_Calculate_Text_Fit_Size(ui_tree, node, text_ref);
    }
}

void NU_Calculate_Sizes(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* gl_contexts, struct Vector* nano_vg_contexts)
{   
    if (ui_tree->deepest_layer == 0) return;

    // For each layer
    for (int l=ui_tree->deepest_layer; l>=0; l--)
    {
        struct Vector* parent_layer = &ui_tree->tree_stack[l];
        struct Vector* child_layer = &ui_tree->tree_stack[l+1];
        
        // Iterate over layer
        for (int p=0; p<parent_layer->size; p++)
        {   
            struct Node* parent = Vector_Get(parent_layer, p);
            int is_layout_horizontal = (parent->layout_flags & 0x01) == LAYOUT_HORIZONTAL;

            if (parent->tag == WINDOW)
            {
                int window_width, window_height;
                SDL_GetWindowSize(parent->window, &window_width, &window_height);
                parent->width = (float) window_width;
                parent->height = (float) window_height;
            }

            if (parent->child_count == 0)
            {
                continue; // Skip acummulating child sizes (no children)
            }
            
            // Track the total width and height for the parent's content
            float content_width = 0;
            float content_height = 0;

            // Iterate over children
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
            {
                struct Node* child = Vector_Get(child_layer, i);

                if (child->tag == WINDOW)
                {
                    if (is_layout_horizontal) content_width -= parent->gap + child->width;
                    if (!is_layout_horizontal) content_height -= parent->gap + child->height;
                }   

                // Dont' accumulate if parent is a window
                if (is_layout_horizontal) // Horizontal Layout
                {
                    content_width += child->width;
                    content_height = MAX(content_height, child->height);
                }

                if (!is_layout_horizontal) // Vertical Layout
                {
                    content_height += child->height;
                    content_width = MAX(content_width, child->width);
                }
            }

            // Grow parent node
            
            if (is_layout_horizontal) content_width += (parent->child_count - 1) * parent->gap;
            if (!is_layout_horizontal) content_height += (parent->child_count - 1) * parent->gap;
            parent->content_width = content_width;
            parent->content_height = content_height;

            if (parent->tag != WINDOW)
            {
                parent->width = content_width + parent->border_left + parent->border_right + parent->pad_left + parent->pad_right;
                parent->height = content_height + parent->border_top + parent->border_bottom + parent->pad_top + parent->pad_bottom;
            }
        }
    }
}

void NU_Grow_Child_Nodes_H(struct Node* parent, struct Vector* child_layer)
{
    float remaining_width = parent->width - parent->pad_left - parent->pad_right - parent->border_left - parent->border_right - ((parent->layout_flags & OVERFLOW_VERTICAL_SCROLL) != 0) * 12.0f;

    if (parent->layout_flags & LAYOUT_VERTICAL)
    {   
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->layout_flags & GROW_HORIZONTAL)
            {
                child->width = remaining_width; 
            }
        }
    }
    else
    {
        uint32_t growable_count = 0;
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->tag == WINDOW) remaining_width += parent->gap;
            else remaining_width -= child->width;
            if (child->layout_flags & GROW_HORIZONTAL && child->tag != WINDOW) growable_count++;
        }

        remaining_width -= (parent->child_count - 1) * parent->gap;

        if (growable_count == 0) return;

        while (remaining_width > 0.1f)
        {
            float smallest = 1e20f;
            for (int i = parent->first_child_index; i < parent->first_child_index + parent->child_count; i++) 
            {
                struct Node* child = Vector_Get(child_layer, i);
                if (child->layout_flags & GROW_HORIZONTAL && child->tag != WINDOW) 
                {
                    smallest = child->width;
                    break;
                }
            }
            float second_smallest = 1e200;
            float width_to_add = remaining_width;

            // for each child
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++) 
            {
                struct Node* child = Vector_Get(child_layer, i);
                
                // if child is growable
                if (child->layout_flags & GROW_HORIZONTAL && child->tag != WINDOW) 
                {
                    if (child->width < smallest)
                    {
                        second_smallest = smallest;
                        smallest = child->width;
                    }
                    if (child->width > smallest)
                    {
                        second_smallest = min(child->width, second_smallest);
                        width_to_add = second_smallest - smallest;
                    }
                }
            }

            width_to_add = min(width_to_add, remaining_width / growable_count);

            bool added_any = false;

            // for each child
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++) 
            {
                struct Node* child = Vector_Get(child_layer, i);
            
                // if child is growable
                if (child->layout_flags & GROW_HORIZONTAL && child->tag != WINDOW) 
                {
                    if (child->width == smallest)
                    {
                        child->width += width_to_add;
                        remaining_width -= width_to_add;
                        added_any = true;
                    }
                }
            }

            if (!added_any) break;
        }
    }
}

void NU_Grow_Child_Nodes_V(struct Node* parent, struct Vector* child_layer)
{
    float remaining_height = parent->height - parent->pad_top - parent->pad_bottom - parent->border_top - parent->border_bottom - ((parent->layout_flags & OVERFLOW_HORIZONTAL_SCROLL) != 0) * 12.0f;

    if (!(parent->layout_flags & LAYOUT_VERTICAL))
    {
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->layout_flags & GROW_VERTICAL)
            {
                child->height = remaining_height; 
            }
        }
    }
    else
    {
        uint32_t growable_count = 0;
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->tag == WINDOW) remaining_height += parent->gap;
            else remaining_height -= child->height;
            if (child->layout_flags & GROW_VERTICAL && child->tag != WINDOW) growable_count++;
        }

        if (growable_count == 0) return;

        while (remaining_height > 0.1f)
        {
            float smallest = 1e20f;
            for (int i = parent->first_child_index; i < parent->first_child_index + parent->child_count; i++) 
            {
                struct Node* child = Vector_Get(child_layer, i);
                if ((child->layout_flags & GROW_VERTICAL) && child->tag != WINDOW) 
                {
                    smallest = child->height;
                    break;
                }
            }
            float second_smallest = 1e200;
            float height_to_add = remaining_height;

            // for each child
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++) 
            {
                struct Node* child = Vector_Get(child_layer, i);
                
                // if child is growable
                if (child->layout_flags & GROW_VERTICAL && child->tag != WINDOW) 
                {
                    if (child->height < smallest)
                    {
                        second_smallest = smallest;
                        smallest = child->height;
                    }
                    if (child->height > smallest)
                    {
                        second_smallest = min(child->height, second_smallest);
                        height_to_add = second_smallest - smallest;
                    }
                }
            }

            height_to_add = min(height_to_add, remaining_height / growable_count);

            bool added_any = false;

            // for each child
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++) 
            {
                struct Node* child = Vector_Get(child_layer, i);
            
                // if child is growable
                if (child->layout_flags & GROW_VERTICAL && child->tag != WINDOW) 
                {
                    if (child->height == smallest)
                    {
                        child->height += height_to_add;
                        remaining_height -= height_to_add;
                        added_any = true;
                    }
                }
            }

            if (!added_any) break;
        }
    }
}

void NU_Grow_Nodes(struct UI_Tree* ui_tree)
{
    // For each layer
    for (int l=0; l<=ui_tree->deepest_layer; l++)
    {
        struct Vector* parent_layer = &ui_tree->tree_stack[l];
        struct Vector* child_layer = &ui_tree->tree_stack[l+1];
        
        // For node in layer
        for (int p=0; p<parent_layer->size; p++)
        {   
            struct Node* parent = Vector_Get(parent_layer, p);

            NU_Grow_Child_Nodes_H(parent, child_layer);
            NU_Grow_Child_Nodes_V(parent, child_layer);
        }
    }
}

void NU_Calculate_Positions(struct UI_Tree* ui_tree)
{
    // Position the root window node
    struct Node* root_window_node = Vector_Get(&ui_tree->tree_stack[0], 0);
    root_window_node->x = 0;
    root_window_node->y = 0;

    if (ui_tree->deepest_layer == 0) return;

    // Calculate the cursor buffer reserve size
    uint32_t cursor_buffer_capacity = 0;
    for (int i=1; i<ui_tree->deepest_layer-1; i++)
    {
        cursor_buffer_capacity = MAX(cursor_buffer_capacity, ui_tree->tree_stack[i].size);
    }

    // Create two cursor buffers (double buffered)
    struct Vector cursor_buffer_0;
    struct Vector cursor_buffer_1;
    Vector_Reserve(&cursor_buffer_0, sizeof(struct NU_Cursor), cursor_buffer_capacity);
    Vector_Reserve(&cursor_buffer_1, sizeof(struct NU_Cursor), cursor_buffer_capacity);

    // Create new cursor for root node and add to second buffer
    struct NU_Cursor root_cursor;
    root_cursor.x = root_window_node->x;
    root_cursor.y = root_window_node->y;
    root_cursor.gap = root_window_node->gap;
    Vector_Push(&cursor_buffer_1, &root_cursor);

    if (ui_tree->deepest_layer > 0)
    {
        // Iterate over each layer
        for (int l=0; l<=ui_tree->deepest_layer; l++)
        {
            int buffer_in_use = l % 2; // Alternate between the two buffers

            // Clear the last buffer in use
            if (buffer_in_use == 0) cursor_buffer_1.size = 0;
            else cursor_buffer_0.size = 0;

            // Iterate over all nodes in layer
            struct Vector* layer = &ui_tree->tree_stack[l];
            for (int n=0; n<layer->size; n++)
            {   
                struct Node* node = Vector_Get(layer, n);
                uint32_t cursor_index = node->parent_index;

                // Get cursor
                struct NU_Cursor* cursor;
                if (buffer_in_use == 0) cursor = (struct NU_Cursor*) Vector_Get(&cursor_buffer_0, cursor_index);
                else cursor = (struct NU_Cursor*) Vector_Get(&cursor_buffer_1, cursor_index);


                // Position node
                if (node->tag == WINDOW)
                {
                    node->x = 0;
                    node->y = 0;
                }
                else
                {
                    node->x = cursor->x + node->border_left;
                    node->y = cursor->y + node->border_top;
                    
                    if (cursor->is_vertical) cursor->y += node->height + cursor->gap;
                    else cursor->x += node->width + cursor->gap;
                }

                if (l != ui_tree->deepest_layer)
                {
                    // Create new cursor for node
                    struct NU_Cursor new_cursor;
                    new_cursor.x = node->x + node->pad_left;
                    new_cursor.y = node->y + node->pad_top;
                    new_cursor.gap = node->gap;
                    new_cursor.is_vertical = node->layout_flags & LAYOUT_VERTICAL;

                    // Add new node cursor
                    if (buffer_in_use == 0) 
                    {
                        Vector_Push(&cursor_buffer_1, &new_cursor);
                    }
                    else
                    {
                        Vector_Push(&cursor_buffer_0, &new_cursor);
                    }
                }
            }
        }
    }

    Vector_Free(&cursor_buffer_0);
    Vector_Free(&cursor_buffer_1);
}
// UI layout ------------------------------------------------------------



// UI rendering ---------------------------------------------------------
void NU_Draw_Node(struct Node* node, NVGcontext* vg)
{
    float inner_width  = node->width - node->border_left - node->border_right - node->pad_left - node->pad_right;
    float inner_height = node->height - node->border_top - node->border_bottom - node->pad_top - node->pad_bottom;

    // Fill color based on tag
    NVGcolor fillColor;
    switch(node->tag)
    {
        case RECT:   fillColor = nvgRGB(120, 100, 100); break;
        case BUTTON: fillColor = nvgRGB(50, 50, 50); break;
        case WINDOW: fillColor = nvgRGB(60, 30, 255); break;
        default:     fillColor = nvgRGB(100, 150, 120); break;
    }

    nvgBeginPath(vg);
    nvgRect(vg, node->x, node->y, node->width, node->height); // could be improved with per-corner radii
    nvgFillColor(vg, fillColor);
    nvgFill(vg);

    // Optional: draw border
    NVGcolor borderColor = nvgRGBA(node->border_r, node->border_g, node->border_b, node->border_a);
    nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);

    // Scrollbars if needed
    if (node->content_height > inner_height && (node->layout_flags & OVERFLOW_VERTICAL_SCROLL)) {
        nvgBeginPath(vg);
        nvgRect(vg, node->x + node->width - 12, node->y, 12, inner_height);
        nvgFillColor(vg, nvgRGB(255,255,255));
        nvgFill(vg);
    }
    if (node->content_width > inner_width && (node->layout_flags & OVERFLOW_HORIZONTAL_SCROLL)) {
        nvgBeginPath(vg);
        nvgRect(vg, node->x, node->y + node->height - 12, inner_width, 12);
        nvgFillColor(vg, nvgRGB(255,255,255));
        nvgFill(vg);
    }
}

void NU_Draw_Node_Text(struct UI_Tree* ui_tree, struct Node* node, char* text,NVGcontext* vg)
{
    struct Vector* font_registry = Vector_Get(&ui_tree->font_registries, 0);
    int fontID = *(int*) Vector_Get(font_registry, 0);
    nvgFontFaceId(node->vg, fontID);   
    nvgFontSize(node->vg, 18);

    nvgFillColor(vg, nvgRGB(255,255,255));
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

    float inner_width  = node->width - node->border_left - node->border_right;
    float inner_height = node->height - node->border_top - node->border_bottom;

    float textBounds[4];
    nvgTextBounds(vg, 0,0, text, NULL, textBounds);
    float textWidth = textBounds[2] - textBounds[0];
    float textHeight = textBounds[3] - textBounds[1];

    float textPosX = node->x + node->border_left + inner_width*0.5f;
    float textPosY = node->y + node->border_top + inner_height*0.5f;

    nvgText(vg, floorf(textPosX), floorf(textPosY), text, NULL);
}

void NU_Render(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* gl_contexts, struct Vector* nano_vg_contexts)
{
    struct Vector window_nodes_list[16];
    for (int i=0; i<windows->size; i++)
    {
        Vector_Reserve(&window_nodes_list[i], sizeof(struct Node*), 1000);
    }

    // For each layer
    for (int l=0; l<=ui_tree->deepest_layer; l++)
    {
        struct Vector* layer = &ui_tree->tree_stack[l];
        for (int n=0; n<layer->size; n++)
        {   
            struct Node* node = Vector_Get(layer, n);
            for (int i=0; i<windows->size; i++)
            {
                SDL_Window* window = *(SDL_Window**) Vector_Get(windows, i);
                if (window == node->window)
                {
                    Vector_Push(&window_nodes_list[i], &node);
                }
            }
        }
    }


    // For each window
    for (int i=0; i<windows->size; i++)
    {
        SDL_Window* window = *(SDL_Window**) Vector_Get(windows, i);
        SDL_GLContext gl_context = *(SDL_GLContext*) Vector_Get(gl_contexts, i);
        NVGcontext* nano_vg_context = *(NVGcontext**) Vector_Get(nano_vg_contexts, i);
        SDL_GL_MakeCurrent(window, gl_context);

        // Clear the window
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        nvgBeginFrame(nano_vg_context, w, h, 1.0f);

        // For each node belonging to the window
        for (int n=0; n<window_nodes_list[i].size; n++)
        {
            struct Node* node = *(struct Node**) Vector_Get(&window_nodes_list[i], n);
            NU_Draw_Node(node, nano_vg_context);

            if (node->text_ref_index != -1)
            {
                struct Text_Ref* text_ref = (struct Text_Ref*) Vector_Get(&ui_tree->text_arena.text_refs, node->text_ref_index);
                
                // Extract pointer to text
                char* text = ui_tree->text_arena.char_buffer.data + text_ref->buffer_index;
                if (text_ref->char_count != text_ref->char_capacity) text[text_ref->char_count] = '\0';
                NU_Draw_Node_Text(ui_tree, node, text, nano_vg_context);
                if (text_ref->char_count != text_ref->char_capacity) text[text_ref->char_count] = ' ';
            }
        }

        // Draw a single test rectangle
        nvgBeginPath(nano_vg_context);
        nvgRoundedRect(nano_vg_context, 50, 50, 200, 100, 10);  // x, y, width, height, radius
        nvgFillColor(nano_vg_context, nvgRGB(255, 0, 0));       // bright red

        // Render to window
        nvgEndFrame(nano_vg_context);
        SDL_GL_SwapWindow(window); 
    }

    for (int i=0; i<windows->size; i++)
    {
        Vector_Free(&window_nodes_list[i]);
    }
}
// UI rendering ---------------------------------------------------------



// Window resize event handling -----------------------------------------
struct NU_Watcher_Data {
    struct UI_Tree* ui_tree;
    struct Vector* windows;
    struct Vector* gl_contexts;
    struct Vector* nano_vg_contexts;
};

bool ResizingEventWatcher(void* data, SDL_Event* event) 
{
    struct NU_Watcher_Data* wd = (struct NU_Watcher_Data*)data;

    if (event->type == SDL_EVENT_WINDOW_RESIZED) 
    {
        NU_Clear_Node_Sizes(wd->ui_tree, wd->windows, wd->gl_contexts, wd->nano_vg_contexts);
        NU_Calculate_Text_Fit_Sizes(wd->ui_tree);
        NU_Calculate_Sizes(wd->ui_tree, wd->windows, wd->gl_contexts, wd->nano_vg_contexts);
        NU_Grow_Nodes(wd->ui_tree);
        NU_Calculate_Positions(wd->ui_tree);
        NU_Render(wd->ui_tree, wd->windows, wd->gl_contexts, wd->nano_vg_contexts);
    }
    return true;
}
// Window resize event handling -----------------------------------------