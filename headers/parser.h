#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_TREE_DEPTH 32
#define LAYOUT_HORIZONTAL 0x00
#define LAYOUT_VERTICAL   0x01

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "performance.h"
#include "vector.h"

const char* keywords[] = {
    "id",
    "dir",
    "grow",
    "window",
    "rect",
    "button",
    "grid",
    "text",
    "image"
};

const uint8_t keyword_lengths[] = { 2, 3, 4, 6, 4, 6, 4, 4, 5 };



// Enums ------------------------ //

enum NU_Token
{
    ID_PROPERTY,
    LAYOUT_DIRECTION_PROPERTY,
    GROW_PROPERTY,
    WINDOW_TAG,
    RECT_TAG,
    BUTTON_TAG,
    GRID_TAG,
    TEXT_TAG,
    IMAGE_TAG,
    OPEN_TAG,
    CLOSE_TAG,
    OPEN_END_TAG,
    CLOSE_END_TAG,
    PROPERTY_ASSIGNMENT,
    PROPERTY_VALUE,
    UNDEFINED
};

enum Tag
{
    WINDOW,
    RECT,
    BUTTON,
    GRID,
    TEXT,
    NAT,
};

// Enums ------------------------ //



// Structs ---------------------- //

struct Node
{
    SDL_Renderer* renderer;
    enum Tag tag;
    float x, y, width, height, gap;
    int parent_index;
    int first_child_index;
    uint16_t child_capacity;
    uint16_t child_count;
    uint16_t pad_top, pad_bottom, pad_left, pad_right;
    uint16_t border_top, border_bottom, border_left, border_right;
    uint16_t border_radius_tl, border_radius_tr, border_radius_bl, border_radius_br;
    char background_r, background_g, background_b, background_a;
    char border_r, border_g, border_b, border_a;
    char layout_flags;
    char growth;
};

struct Property_Text_Ref
{
    uint32_t NU_Token_index;
    uint32_t src_index;
    uint8_t char_count;
};

struct Text_Ref
{
    uint32_t buffer_index;
    uint32_t char_count;
    uint32_t char_capacity;
};

struct Arena_Free_Element
{
    uint32_t index;
    uint32_t length;
};

struct Text_Arena
{
    struct Vector free_list;
    struct Vector text_refs;
    struct Vector char_buffer;
};

struct UI_Tree
{
    struct Vector tree_stack[MAX_TREE_DEPTH];
    struct Text_Arena text_arena;
    uint16_t deepest_layer;
};

// Structs ---------------------- //



// Internal Functions ----------- //

static enum NU_Token NU_Word_To_NU_Token(char word[], uint8_t word_char_count)
{
    for (uint8_t i = 0; i < 9; i++) {
        size_t len = keyword_lengths[i];
        if (len == word_char_count && memcmp(word, keywords[i], len) == 0) {
            return i;
        }
    }
    return UNDEFINED;
}

static enum Tag NU_Token_To_Tag(enum NU_Token NU_Token)
{
    int tag_candidate = NU_Token - 3;
    if (tag_candidate < 0) return NAT;
    return NU_Token - 3;
}

static void NU_Tokenise(char* src_buffer, uint32_t src_length, struct Vector* NU_Token_vector, struct Vector* ptext_ref_vector, struct Text_Arena* text_arena) 
{
    // Store current NU_Token word
    uint8_t word_char_index = 0;
    char word[24];

    // Store global text char indexes
    uint32_t text_arena_buffer_index = 0;
    uint32_t text_char_count = 0;

    // Context
    uint8_t ctx = 0; // 0 == globalspace, 1 == commentspace, 2 == tagspace, 3 == propertyspace

    // Iterate over src file 
    uint32_t i = 0;
    while (i < src_length)
    {
        char c = src_buffer[i];

        // Comment begins
        if (i < src_length - 3 && c == '<' && src_buffer[i+1] == '!' && src_buffer[i+2] == '-' && src_buffer[i+3] == '-' && ctx == 0)
        {
            ctx = 1;
            i+=4;
            continue;
        }

        // Comment ends
        if (ctx == 1 && i < src_length - 2 && c == '-' && src_buffer[i+1] == '-' && src_buffer[i+2] == '>')
        {
            ctx = 0;
            i+=3;
            continue;
        }

        // In comment
        if (ctx == 1)
        {
            i+=1;
            continue;
        }

        // Open end tag
        if (ctx == 0 && i < src_length - 1 && c == '<' && src_buffer[i+1] == '/')
        {
            // Reset global text character count
            if (text_char_count > 0)
            {
                struct Text_Ref new_ref;
                new_ref.char_count = text_char_count;
                new_ref.char_capacity = text_char_count;
                new_ref.buffer_index = text_arena_buffer_index;
                Vector_Push(&text_arena->text_refs, &new_ref);
            }
            text_char_count = 0;

            enum NU_Token t = OPEN_END_TAG;
            Vector_Push(NU_Token_vector, &t);
            word_char_index = 0;
            ctx = 2;
            i+=2;
            continue;
        }

        // Tag begins
        if (ctx == 0 && c == '<')
        {
            // Reset global text character count
            if (text_char_count > 0)
            {
                struct Text_Ref new_ref;
                new_ref.char_count = text_char_count;
                new_ref.char_capacity = text_char_count;
                new_ref.buffer_index = text_arena_buffer_index;
                Vector_Push(&text_arena->text_refs, &new_ref);
            }
            text_char_count = 0;

            enum NU_Token t = OPEN_TAG;
            Vector_Push(NU_Token_vector, &t);
            word_char_index = 0;
            ctx = 2;
            i+=1;
            continue;
        }

        // Self closing end tag
        if (ctx == 2 && i < src_length - 1 && c == '/' && src_buffer[i+1] == '>')
        {
            if (word_char_index > 0) {
                enum NU_Token t = NU_Word_To_NU_Token(word, word_char_index);
                Vector_Push(NU_Token_vector, &t);
            }
            enum NU_Token t = CLOSE_END_TAG;
            Vector_Push(NU_Token_vector, &t);
            word_char_index = 0;
            ctx = 0;
            i+=2;
            continue;
        }

        // Tag ends
        if (ctx == 2 && c == '>')
        {
            if (word_char_index > 0) {
                enum NU_Token t = NU_Word_To_NU_Token(word, word_char_index);
                Vector_Push(NU_Token_vector, &t);
            }
            enum NU_Token t = CLOSE_TAG;
            Vector_Push(NU_Token_vector, &t);
            word_char_index = 0;
            ctx = 0;
            i+=1;
            continue;
        }

        // Property assignment
        if (ctx == 2 && c == '=')
        {
            if (word_char_index > 0) {
                enum NU_Token t = NU_Word_To_NU_Token(word, word_char_index);
                Vector_Push(NU_Token_vector, &t);
            }
            word_char_index = 0;
            enum NU_Token t = PROPERTY_ASSIGNMENT;
            Vector_Push(NU_Token_vector, &t);
            i+=1;
            continue;
        }

        // Property value begins
        if (ctx == 2 && c == '"')
        {
            ctx = 3;
            i+=1;
            continue;
        }

        // Property value ends
        if (ctx == 3 && c == '"')
        {
            enum NU_Token t = PROPERTY_VALUE;
            Vector_Push(NU_Token_vector, &t);
            if (word_char_index > 0)
            {
                struct Property_Text_Ref ref;
                ref.NU_Token_index = NU_Token_vector->size - 1;
                ref.src_index = i - word_char_index;
                ref.char_count = word_char_index;
                Vector_Push(ptext_ref_vector, &ref);
            }
            word_char_index = 0;
            ctx = 2;
            i+=1;
            continue;
        }

        // If split character
        if (c == ' ' || c == '\t' || c == '\n')
        {
            if (word_char_index > 0) {
                enum NU_Token t = NU_Word_To_NU_Token(word, word_char_index);
                Vector_Push(NU_Token_vector, &t);
            }
            word_char_index = 0;
            i+=1;
            continue;
        }

        // If tag space or property space
        if (ctx == 2 || ctx == 3)
        {
            word[word_char_index] = c;
            word_char_index+=1;
        }

        // If global space and split character
        if (ctx == 0 && c != '\t' && c != '\n')
        {
            // text continues
            if (text_char_count > 0)
            {
                Vector_Push(&text_arena->char_buffer, &c);
                text_char_count += 1;
            }

            // text starts
            if (text_char_count == 0 && c != ' ')
            {
                text_arena_buffer_index = text_arena->char_buffer.size;
                Vector_Push(&text_arena->char_buffer, &c);
                text_char_count = 1;
            }
        }

        i+=1;
    }

    if (word_char_index > 0) {
        enum NU_Token t = NU_Word_To_NU_Token(word, word_char_index);
        Vector_Push(NU_Token_vector, &t);
    }
}

static int NU_Is_Token_Property(enum NU_Token NU_Token)
{
    return NU_Token < 3;
}

static int AssertRootGrammar(struct Vector* token_vector)
{
    // ENFORCE RULE: FIRST TOKEN MUST BE OPEN TAG
    // ENFORCE RULE: SECOND TOKEN MUST BE TAG NAME
    // ENFORCE RULE: THIRD TOKEN MUST BE CLOSE_TAG | PROPERTY
    int root_open = token_vector->size > 2 && 
        *((enum NU_Token*) Vector_Get(token_vector, 0)) == OPEN_TAG &&
        *((enum NU_Token*) Vector_Get(token_vector, 1)) == WINDOW_TAG;

    if (token_vector->size > 2 &&
        *((enum NU_Token*) Vector_Get(token_vector, 0)) == OPEN_TAG &&
        *((enum NU_Token*) Vector_Get(token_vector, 1)) == WINDOW_TAG) 
    {
        if (token_vector->size > 5 && 
            *((enum NU_Token*) Vector_Get(token_vector, token_vector->size-3)) == OPEN_END_TAG &&
            *((enum NU_Token*) Vector_Get(token_vector, token_vector->size-2)) == WINDOW_TAG && 
            *((enum NU_Token*) Vector_Get(token_vector, token_vector->size-1)) == CLOSE_TAG)
        {
            return 0; // Success
        }
        else // Closing tag is not window
        {
            printf("%s\n", "[Generate_Tree] Error! XML tree root not closed.");
            return -1;
        }
    }
    else // Root is not a window tag
    {
        printf("%s\n", "[Generate_Tree] Error! XML tree has no root. XML documents must begin with a <window> tag.");
        return -1;
    }
}

static int AssertNewTagGrammar(struct Vector* token_vector, int i)
{
    // ENFORCE RULE: NEXT TOKEN SHOULD BE TAG NAME 
    if (i < token_vector->size - 2 && NU_Token_To_Tag(*((enum NU_Token*) Vector_Get(token_vector, i+1))) != NAT)
    {
        enum NU_Token third_token = *((enum NU_Token*) Vector_Get(token_vector, i+2));
        if (third_token == CLOSE_TAG || third_token == CLOSE_END_TAG || NU_Is_Token_Property(third_token)) return 0; // Success
    }
    return -1; // Failure
}

static int AssertTagCloseStartGrammar(struct Vector* token_vector, int i, enum Tag openTag)
{
    // ENFORCE RULE: NEXT TOKEN SHOULD BE TAG AND MUST MATCH OPENING TAG
    // ENDORCE RULE: THIRD TOKEN MUST BE A TAG END
    if (i < token_vector->size - 2 && 
        NU_Token_To_Tag(*((enum NU_Token*) Vector_Get(token_vector, i+1))) == openTag && 
        *((enum NU_Token*) Vector_Get(token_vector, i+2)) == CLOSE_TAG) return 0; // Success
    else {
        printf("%s", "[Generate Tree] Error! Closing tag does not match.");
        printf("%s %d %s %d", "close tag:", NU_Token_To_Tag(*((enum NU_Token*) Vector_Get(token_vector, i+1))), "open tag:", openTag);
    }
    return -1; // Failure
}

static int NU_Generate_Tree(char* src_buffer, uint32_t src_length, struct UI_Tree* ui_tree, struct Vector* NU_Token_vector, struct Vector* ptext_ref_vector)
{
    // Enforce root grammar
    if (AssertRootGrammar(NU_Token_vector) != 0) {
        return -1; // Failure 
    }

    // Iterate over all NU_Tokens
    int i = 0;
    int current_layer = -1;
    ui_tree->deepest_layer = 0;
    while (i < NU_Token_vector->size - 3)
    {
        const enum NU_Token NU_Token = *((enum NU_Token*) Vector_Get(NU_Token_vector, i));

        // New tag -> Add a new node
        if (NU_Token == OPEN_TAG)
        {
            if (AssertNewTagGrammar(NU_Token_vector, i) == 0)
            {
                // Enforce max tree depth
                if (current_layer+1 == MAX_TREE_DEPTH)
                {
                    printf("%s", "[Generate Tree] Error! Exceeded max tree depth of 64");
                    return -1; // Failure
                }

                // Create a new node
                struct Node newNode;
                newNode.tag = NU_Token_To_Tag(*((enum NU_Token*) Vector_Get(NU_Token_vector, i+1)));
                newNode.renderer = NULL; 
                newNode.child_count = 0;
                newNode.first_child_index = -1;
                newNode.parent_index = ui_tree->tree_stack[current_layer].size - 1;
                Vector_Push(&ui_tree->tree_stack[current_layer+1], &newNode);


                if (current_layer != -1) // Only equals -1 for the root window node (optimisation would be to remove this check and append root node at beggining of function)
                {
                    // Inform parent that parent has new child
                    struct Node* parentNode = (struct Node*) Vector_Get(&ui_tree->tree_stack[current_layer], newNode.parent_index);
                    if (parentNode->child_count == 0)
                    {
                        parentNode->first_child_index = ui_tree->tree_stack[current_layer+1].size - 1;
                    }
                    parentNode->child_count += 1;
                    parentNode->child_capacity += 1;
                }

                

                // Move one layer deeper
                current_layer++;

                ui_tree->deepest_layer = MAX(ui_tree->deepest_layer, current_layer);

                // Increment NU_Token
                i+=2;
                continue;
            }
            else
            {
                return -1;
            }
        }

        // Open end tag -> tag closes -> move up one layer
        if (NU_Token == OPEN_END_TAG)
        {
            if (current_layer < 0) break;
            
            // Check close grammar
            enum Tag openTag = ((struct Node*) Vector_Get(&ui_tree->tree_stack[current_layer], ui_tree->tree_stack[current_layer].size - 1))->tag;
            if (AssertTagCloseStartGrammar(NU_Token_vector, i, openTag) != 0)
            {
                return -1; // Failure
            }

            // Move one layer higher
            current_layer--;

            // Increment NU_Token
            i+=3;
            continue;
        }

        // Close end tag -> tag self closes -> move up one layer
        if (NU_Token == CLOSE_END_TAG)
        {
            // Move one layer higher
            current_layer--;

            // Increment NU_Token
            i+=1;
            continue;
        }
        
        i+= 1;
    }

    return 0; // Success
}

// Internal Functions ----------- //



// Public Functions ------------- //

int NU_Parse(char* filepath, struct UI_Tree* ui_tree)
{
    // Open XML source file
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Cannot open file '%s': %s\n", filepath, strerror(errno));
        return -1;
    }

    // Determine file size
    fseek(f, 0, SEEK_END);
    long file_size_long = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (file_size_long > UINT32_MAX) 
    {
        printf("%s", "Src file is too large! It must be < 4 294 967 295 Bytes");
        return -1;
    }

    // Create a src buffer and calculate the length
    uint32_t src_length = (uint32_t)file_size_long;
    char* src_buffer = malloc(src_length + 1);
    src_length = fread(src_buffer, 1, file_size_long, f);
    src_buffer[src_length] = '\0'; 
    fclose(f);

    // Init Token vector and reserve ~1MB
    struct Vector NU_Token_vector;
    Vector_Reserve(&NU_Token_vector, sizeof(enum NU_Token), 250000);

    // Init property text reference vector and reserve ~900KB
    struct Vector ptext_ref_vector;
    Vector_Reserve(&ptext_ref_vector, sizeof(struct Property_Text_Ref), 100000);

    // Init UI tree text arena
    Vector_Reserve(&ui_tree->text_arena.free_list, sizeof(struct Arena_Free_Element), 100000); // reserve ~800KB
    Vector_Reserve(&ui_tree->text_arena.text_refs, sizeof(struct Text_Ref), 100000); // reserve ~800KB
    Vector_Reserve(&ui_tree->text_arena.char_buffer, sizeof(char), 1000000); // reserve ~1MB

    // Init UI tree layers -> reserve 100 nodes per stack layer = 384KB
    Vector_Reserve(&ui_tree->tree_stack[0], sizeof(struct Node), 1); // 1 root element
    for (int i=1; i<MAX_TREE_DEPTH; i++) {
        Vector_Reserve(&ui_tree->tree_stack[i], sizeof(struct Node), 100);
    }

    // Tokenise the file source
    NU_Tokenise(src_buffer, src_length, &NU_Token_vector, &ptext_ref_vector, &ui_tree->text_arena);

    // Generate UI tree
    if (NU_Generate_Tree(src_buffer, src_length, ui_tree, &NU_Token_vector, &ptext_ref_vector) != 0) return -1; // Failure

    // Free token and property text reference memory
    Vector_Free(&NU_Token_vector);
    Vector_Free(&ptext_ref_vector);

    return 0; // Success
}

void NU_Free_UI_Tree_Memory(struct UI_Tree* ui_tree)
{
    Vector_Free(&ui_tree->text_arena.free_list);
    Vector_Free(&ui_tree->text_arena.text_refs);
    Vector_Free(&ui_tree->text_arena.char_buffer);
}

// Public Functions ------------- //