#include <SDL3/SDL.h>

struct NU_Cursor
{
    float x, y, gap;
};

void NU_Create_New_Window(struct Node* window_node, struct Vector* windows, struct Vector* renderers)
{
    SDL_Window* new_window = SDL_CreateWindow("window", 500, 400, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* new_renderer = SDL_CreateRenderer(new_window, "direct3d");
    Vector_Push(windows, &new_window);
    Vector_Push(renderers, &new_renderer);
    window_node->renderer = new_renderer;
}

void NU_Reset_Node_size(struct Node* node)
{
    node->border_left = 0;
    node->border_right = 0;
    node->border_top = 0;
    node->border_bottom = 0;
    node->pad_left = 4;
    node->pad_right = 4;
    node->pad_top = 4;
    node->pad_bottom = 4;
    node->gap = 10.0f;
    node->width = 0;
    node->height = 0;
}

void NU_Calculate_Sizes(struct UI_Tree* ui_tree, struct Vector* windows, struct Vector* renderers)
{   
    // For each node in deepest layer
    struct Vector* deepest_layer = &ui_tree->tree_stack[ui_tree->deepest_layer];
    for (int i=0; i<deepest_layer->size; i++)
    {   
        struct Node* node = Vector_Get(deepest_layer, i);
        NU_Reset_Node_size(node);
        node->width = 50;
        node->height = 50;

        // If parent is window node and has no SDL window assigned to it
        if (node->tag == WINDOW && node->renderer == NULL)
        {
            // Create a new window and renderer
            NU_Create_New_Window(node, windows, renderers);
        }

        // Inherit window and renderer from parent
        if (node->tag != WINDOW && node->renderer == NULL)
        {
            struct Node* parent = Vector_Get(&ui_tree->tree_stack[ui_tree->deepest_layer - 1], node->parent_index);
            node->renderer = parent->renderer;
        }
    }

    if (ui_tree->deepest_layer == 0) return;

    // For each layer exluding the deepest
    for (int l=ui_tree->deepest_layer-1; l>=0; l--)
    {
        struct Vector* parent_layer = &ui_tree->tree_stack[l];
        struct Vector* child_layer = &ui_tree->tree_stack[l+1];
        
        // Iterate over layer
        for (int p=0; p<parent_layer->size; p++)
        {   
            struct Node* parent = Vector_Get(parent_layer, p);
            int is_layout_horizontal = (parent->layout_flags & 0x01) == LAYOUT_HORIZONTAL;
            
            NU_Reset_Node_size(parent);
            parent->width = 50;
            parent->height = 50;

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
            
            parent->width = 0;
            parent->height = 0;

            // Track the total width and height for the parent's content
            float contentWidth = 0;
            float contentHeight = 0;

            // Iterate over children
            for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
            {
                struct Node* child = Vector_Get(child_layer, i);

                // Inherit window and renderer from parent
                if (child->tag != WINDOW && child->renderer == NULL)
                {
                    child->renderer = parent->renderer;
                }

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
            }

            if (parent->tag != NAT)
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

void NU_Grow_Child_Nodes_H(struct Node* parent, struct Vector* child_layer)
{
    float remaining_width = parent->width;

    if (!(parent->layout_flags & LAYOUT_HORIZONTAL))
    {   
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->layout_flags & GROW_HORIZONTAL)
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
            if (child->layout_flags & GROW_HORIZONTAL && child->tag != WINDOW) growable_count += 1;
        }

        if (growable_count == 0) return;

        float growth_per_element = remaining_width / (float) growable_count;

        // Grow child elements
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->layout_flags & GROW_HORIZONTAL && child->tag != WINDOW)
            {
                child->width += growth_per_element;
                child->width = roundf(child->width);
            }
        }
    }
}

void NU_Grow_Child_Nodes_V(struct Node* parent, struct Vector* child_layer)
{
    float remaining_height = parent->height;

    if (parent->layout_flags & LAYOUT_HORIZONTAL)
    {
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->layout_flags & GROW_VERTICAL)
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
            if (child->layout_flags & GROW_VERTICAL && child->tag != WINDOW) growable_count += 1;
        }

        if (growable_count == 0) return;

        float growth_per_element = remaining_height / (float) growable_count;

        // Grow child elements
        for (int i=parent->first_child_index; i<parent->first_child_index + parent->child_count; i++)
        {
            struct Node* child = Vector_Get(child_layer, i);
            if (child->layout_flags & GROW_VERTICAL && child->tag != WINDOW)
            {
                child->height += growth_per_element;
                child->height = roundf(child->height);
            }
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
        // Iterate over each layer excluding the first and last
        for (int l=1; l<=ui_tree->deepest_layer; l++)
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
                    cursor->y += node->height + cursor->gap;
                }

                if (l != ui_tree->deepest_layer)
                {
                    // Create new cursor for node
                    struct NU_Cursor new_cursor;
                    new_cursor.x = node->x + node->pad_left;
                    new_cursor.y = node->y + node->pad_top;
                    new_cursor.gap = node->gap;

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

void NU_Draw_Node(struct Node* node)
{
    if (node->tag == WINDOW) return;

    SDL_FRect rect; 
    rect.x = node->x;
    rect.y = node->y;
    rect.w = node->width;
    rect.h = node->height;

    // printf("%s %d %s %f %f %f %f %s \n", "node tag", node->tag, "position: {", rect.x, rect.y, rect.w, rect.h, "}");

    // SET RECT COLOUR
    if (node->tag == RECT)
    {
        SDL_SetRenderDrawColor(node->renderer, 255, 100, 0, 255);
    }
    else if (node->tag == BUTTON)
    {
        SDL_SetRenderDrawColor(node->renderer, 20, 255, 120, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(node->renderer, 100, 150, 120, 255);
    }
    

    // RENDER RECT
    SDL_RenderFillRect(node->renderer, &rect);
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
        }
    }

    // Render to windows
    for (int i=0; i<renderers->size; i++)
    {
        SDL_Renderer* renderer = *(SDL_Renderer**) Vector_Get(renderers, i);
        SDL_RenderPresent(renderer);
    }
}
