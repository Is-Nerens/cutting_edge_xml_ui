#include <SDL3/SDL.h>
#include <SDL_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>

// UI layout ------------------------------------------------------------
struct NU_Cursor
{
    float x, y, gap;
    char is_vertical;
};

void NU_Create_New_Window(struct Node* window_node, struct Vector* windows, struct Vector* renderers)
{
    SDL_Window* new_window = SDL_CreateWindow("window", 500, 400, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* new_renderer = SDL_CreateRenderer(new_window, "opengl");
    Vector_Push(windows, &new_window);
    Vector_Push(renderers, &new_renderer);
    window_node->renderer = new_renderer;
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

void NU_Clear_Node_Sizes(struct UI_Tree* ui_tree)
{
    // For each layer
    for (int l=0; l<=ui_tree->deepest_layer; l++)
    {
        struct Vector* layer = &ui_tree->tree_stack[l];
        
        // Draw all nodes in layer
        for (int n=0; n<layer->size; n++)
        {   
            struct Node* node = Vector_Get(layer, n);
            NU_Reset_Node_size(node);
        }
    }
}

void NU_Calculate_Text_Fit_Size(struct UI_Tree* ui_tree, struct Node* node, struct Text_Ref* text_ref)
{
    // Extract pointer to text
    char* text = ui_tree->text_arena.char_buffer.data + text_ref->buffer_index;

    int width = 0, height = 0;
    TTF_GetStringSize(ui_tree->font, text, text_ref->char_count, &width, &height);
    width *= 0.5f;
    height *= 0.5f;

    // Apply size
    if (node->preferred_width == 0.0f) node->width += (float) width;
    node->height += (float) height;
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

void NU_Calculate_Sizes(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* renderers)
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
                SDL_GetWindowSize(SDL_GetRenderWindow(parent->renderer), &window_width, &window_height);
                parent->width = (float) window_width;
                parent->height = (float) window_height;
            }


            // If parent is window node and has no SDL window assigned to it
            if (parent->tag == WINDOW && parent->renderer == NULL)
            {
                // Create a new window and renderer
                NU_Create_New_Window(parent, windows, renderers);
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

                // Inherit renderer from parent
                if (child->tag != WINDOW && child->renderer == NULL)
                {
                    child->renderer = parent->renderer;
                }

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
void NU_Draw_Node(struct Node* node)
{
    float inner_width = node->width - node->border_left - node->border_right - node->pad_left - node->pad_right;
    float inner_height = node->height - node->border_top - node->border_bottom - node->pad_top - node->pad_bottom;
    
    SDL_FRect rect; 
    rect.x = node->x;
    rect.y = node->y;
    rect.w = node->width;
    rect.h = node->height;

    if (node->content_height > inner_height && node->layout_flags & OVERFLOW_VERTICAL_SCROLL)
    {
        rect.w -= 12;

        // Draw horizontal scroll bar
        SDL_FRect scroll_rect;
        scroll_rect.x = rect.x + rect.w - node->border_right;
        scroll_rect.y = rect.y;
        scroll_rect.w = 12;
        scroll_rect.h = rect.h - node->border_bottom;
        SDL_SetRenderDrawColor(node->renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(node->renderer, &scroll_rect);
    }

    if (node->content_width > inner_width && node->layout_flags & OVERFLOW_HORIZONTAL_SCROLL)
    {
        rect.h -= 12;

        // Draw horizontal scroll bar
        SDL_FRect scroll_rect;
        scroll_rect.x = rect.x;
        scroll_rect.y = rect.y + rect.h - node->border_bottom;
        scroll_rect.w = rect.w - node->border_right;
        scroll_rect.h = 12;
        SDL_SetRenderDrawColor(node->renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(node->renderer, &scroll_rect);
    }

    // SET RECT COLOUR
    if (node->tag == RECT)
    {
        SDL_SetRenderDrawColor(node->renderer, 120, 100, 100, 255);
    }
    else if (node->tag == BUTTON)
    {
        SDL_SetRenderDrawColor(node->renderer, 200, 200, 200, 255);
    }
    else if (node->tag == WINDOW)
    {
        SDL_SetRenderDrawColor(node->renderer, 60, 30, 255, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(node->renderer, 100, 150, 120, 255);
    }
    

    // Render Rect
    SDL_RenderFillRect(node->renderer, &rect);

    SDL_FColor col = {
        // (float)node->border_r / 255.0f,
        // (float)node->border_g / 255.0f,
        // (float)node->border_b / 255.0f,
        // (float)node->border_a / 255.0f
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    // Construct Border
    SDL_Vertex verts[8] = {
        { .position = {rect.x, rect.y}, .color = col, .tex_coord = {0, 0} },
        { .position = {rect.x + rect.w, rect.y}, .color = col, .tex_coord = {0, 0} },
        { .position = {rect.x + node->border_left, rect.y + node->border_top}, .color = col, .tex_coord = {0, 0} },
        { .position = {rect.x + rect.w - node->border_right, rect.y + node->border_top}, .color = col, .tex_coord = {0, 0}},
        { .position = {rect.x, rect.y + rect.h}, .color = col, .tex_coord = {0, 0}},
        { .position = {rect.x + rect.w, rect.y + rect.h}, .color = col, .tex_coord = {0, 0}},
        { .position = {rect.x + node->border_left, rect.y + rect.h - node->border_bottom}, .color = col, .tex_coord = {0, 0}},
        { .position = {rect.x + rect.w - node->border_right, rect.y + rect.h - node->border_bottom}, .color = col, .tex_coord = {0, 0}},
    };

    int indices[] = {
        0, 1, 2, 2, 1, 3,      // top border
        1, 5, 7, 1, 7, 3,      // right border
        4, 6, 7, 4, 7, 5,      // bottom border
        4, 0, 6, 0, 2, 6       // left border
    };

    SDL_RenderGeometry(node->renderer, NULL, verts, 8, indices, 24);
}

void NU_Draw_Node_Text(struct UI_Tree* ui_tree, struct Node* node, char* text)
{
    SDL_Color textCol = { 0, 0, 0, 255 };
    SDL_Surface* text_surface = TTF_RenderText_Blended(ui_tree->font, text, strlen(text), textCol);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(node->renderer, text_surface);  
    SDL_SetTextureBlendMode(text_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(text_texture, SDL_SCALEMODE_LINEAR);
    float textWidth = (float) (text_surface->w) * 0.5f;
    float textHeight = (float) (text_surface->h) * 0.5f;
    float half_rem_inner_width = (node->width - node->border_left - node->border_right - textWidth) * 0.5f;
    float half_rem_inner_height = (node->height - node->border_top - node->border_bottom - textHeight) * 0.5f;
    float textPosX = node->x + node->border_left + half_rem_inner_width;
    float textPosY = node->y + node->border_top + half_rem_inner_height;
    SDL_FRect text_rect; 
    text_rect.x = textPosX;
    text_rect.y = textPosY;
    text_rect.w = textWidth;
    text_rect.h = textHeight;
    SDL_RenderTexture(node->renderer, text_texture, NULL, &text_rect);  
    SDL_DestroyTexture(text_texture);
    SDL_DestroySurface(text_surface);
}

void NU_Render(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* renderers)
{
    // Clear window images
    for (int i=0; i<renderers->size; i++)
    {
        SDL_Renderer* renderer = *(SDL_Renderer**) Vector_Get(renderers, i);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  
        SDL_RenderClear(renderer);
    }
        
    // For each layer
    for (int l=0; l<=ui_tree->deepest_layer; l++)
    {
        struct Vector* layer = &ui_tree->tree_stack[l];
        
        // Draw all nodes in layer
        for (int n=0; n<layer->size; n++)
        {   
            struct Node* node = Vector_Get(layer, n);
            NU_Draw_Node(node);

            if (node->text_ref_index != -1)
            {
                struct Text_Ref* text_ref = (struct Text_Ref*) Vector_Get(&ui_tree->text_arena.text_refs, node->text_ref_index);
                
                // Extract pointer to text
                char* text = ui_tree->text_arena.char_buffer.data + text_ref->buffer_index;
                if (text_ref->char_count != text_ref->char_capacity) text[text_ref->char_count] = '\0';
                NU_Draw_Node_Text(ui_tree, node, text);
                if (text_ref->char_count != text_ref->char_capacity) text[text_ref->char_count] = ' ';
            }
        }
    }

    // Render to windows
    for (int i=0; i<renderers->size; i++)
    {
        SDL_Renderer* renderer = *(SDL_Renderer**) Vector_Get(renderers, i);
        SDL_RenderPresent(renderer);
    }
}
// UI rendering ---------------------------------------------------------



// Window resize event handling -----------------------------------------
struct NU_Watcher_Data {
    struct UI_Tree* ui_tree;
    struct Vector* windows;
    struct Vector* renderers;
};

bool ResizingEventWatcher(void* data, SDL_Event* event) 
{
    struct NU_Watcher_Data* wd = (struct NU_Watcher_Data*)data;

    if (event->type == SDL_EVENT_WINDOW_RESIZED) 
    {
        NU_Clear_Node_Sizes(wd->ui_tree);
        NU_Calculate_Text_Fit_Sizes(wd->ui_tree);
        NU_Calculate_Sizes(wd->ui_tree, wd->windows, wd->renderers);
        NU_Grow_Nodes(wd->ui_tree);
        NU_Calculate_Positions(wd->ui_tree);
        NU_Render(wd->ui_tree, wd->windows, wd->renderers);
    }
    return true;
}
// Window resize event handling -----------------------------------------