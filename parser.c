#define _CRT_SECURE_NO_WARNINGS 
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "performance.h"


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

enum Token
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

struct Node
{
    enum Tag tag;
    int parent_index;
    int first_child_index;
    float x, y, width, height;
    uint16_t pad_top, pad_bottom, pad_left, pad_right;
    uint16_t border_top, border_bottom, border_left, border_right;
    uint16_t border_radius_tl, border_radius_tr, border_radius_bl, border_radius_br;
    uint16_t child_count;
    char background_r, background_g, background_b, background_a;
    char border_r, border_g, border_b, border_a;
    char layout_flags;
};

struct PTextRef
{
    uint32_t token_index;
    uint32_t src_index;
    uint8_t char_count;
};

struct Vector
{
    uint32_t capacity;
    uint32_t size;
    void* data;
    size_t element_size;
};

void Vector_Reserve(struct Vector* vector, size_t element_size, uint32_t capacity)
{
    vector->capacity = capacity;
    vector->size = 0;
    vector->element_size = element_size;
    vector->data = malloc(capacity * element_size);
}

void Vector_Free(struct Vector* vector)
{
    free(vector->data);
    vector->data = NULL;
    vector->capacity = 0;
    vector->size = 0;
}

void Vector_Grow(struct Vector* vector)
{
    vector->capacity = MAX(vector->capacity * 2, 2);
    vector->data = realloc(vector->data, vector->capacity * vector->element_size);
}

void Vector_Push(struct Vector* vector, const void* element)
{
    if (vector->size == vector->capacity) {
        Vector_Grow(vector);
    }
    void* destination = (char*)vector->data + vector->size * vector->element_size;
    memcpy(destination, element, vector->element_size);
    vector->size += 1;
}

void* Vector_Get(struct Vector* vector, uint32_t index)
{
    return (char*) vector->data + index * vector->element_size;
}

enum Token word_to_token(char word[], uint8_t word_char_count)
{
    for (uint8_t i = 0; i < 9; i++) {
        size_t len = keyword_lengths[i];
        if (len == word_char_count && memcmp(word, keywords[i], len) == 0) {
            return i;
        }
    }
    return UNDEFINED;
}

void tokenise(char* src_buffer, uint32_t src_length, struct Vector* token_vector, struct Vector* ptext_ref_vector)
{
    // Store current token word
    uint8_t word_char_index = 0;
    char word[24];

    // Context
    uint8_t ctx = 0; // 0 == globalspace, 1 == commentspace, 2 == tagspace, 3 == propertyspace

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

        // Tag begins
        if (ctx == 0 && i < src_length - 1 && c == '<' && src_buffer[i+1] == '/')
        {
            // push tag text if any -> not implemented
            enum Token t = OPEN_END_TAG;
            Vector_Push(token_vector, &t);
            word_char_index = 0;
            ctx = 2;
            i+=2;
            continue;
        }
        if (ctx == 0 && c == '<')
        {
            // push tag text if any -> not implemented
            enum Token t = OPEN_TAG;
            Vector_Push(token_vector, &t);
            word_char_index = 0;
            ctx = 2;
            i+=1;
            continue;
        }

        // Tag ends
        if (ctx == 2 && c == '>')
        {
            if (word_char_index > 0) {
                enum Token t = word_to_token(word, word_char_index);
                Vector_Push(token_vector, &t);
            }
            enum Token t = CLOSE_TAG;
            Vector_Push(token_vector, &t);
            word_char_index = 0;
            ctx = 0;
            i+=1;
            continue;
        }

        if (ctx == 2 && i < src_length - 1 && c == '/' && src_buffer[i+1] == '>')
        {
            if (word_char_index > 0) {
                enum Token t = word_to_token(word, word_char_index);
                Vector_Push(token_vector, &t);
            }
            enum Token t = CLOSE_END_TAG;
            Vector_Push(token_vector, &t);
            word_char_index = 0;
            ctx = 0;
            i+=1;
            continue;
        }

        // Property assignment
        if (ctx == 2 && c == '=')
        {
            if (word_char_index > 0) {
                enum Token t = word_to_token(word, word_char_index);
                Vector_Push(token_vector, &t);
            }
            word_char_index = 0;
            enum Token t = PROPERTY_ASSIGNMENT;
            Vector_Push(token_vector, &t);
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
            enum Token t = PROPERTY_VALUE;
            Vector_Push(token_vector, &t);
            if (word_char_index > 0)
            {
                struct PTextRef ref;
                ref.token_index = token_vector->size - 1;
                ref.src_index = i - word_char_index;
                ref.char_count = word_char_index;
                Vector_Push(ptext_ref_vector, &ref);
            }
            word_char_index = 0;
            ctx = 2;
            i+=1;
            continue;
        }

        if (ctx == 2 && (c == ' ' || c == '\t' || c == '\n'))
        {
            if (word_char_index > 0) {
                enum Token t = word_to_token(word, word_char_index);
                Vector_Push(token_vector, &t);
            }
            word_char_index = 0;
            i+=1;
            continue;
        }

        if (ctx == 2 || ctx == 3)
        {
            word[word_char_index] = c;
            word_char_index+=1;
            i+=1;
            continue;
        }

        else
        {
            i+=1;
        }
    }

    if (word_char_index > 0) {
        enum Token t = word_to_token(word, word_char_index);
        Vector_Push(token_vector, &t);
    }
}

enum Tag TokenToTag(enum Token token)
{
    int tag_candidate = token - 3;
    if (tag_candidate < 0) return NAT;
    return token - 3;
}

int isPropertyToken(enum Token token)
{
    return token < 3;
}

int AssertRootGrammar(struct Vector* token_vector)
{
    // ENFORCE RULE: FIRST TOKEN MUST BE OPEN TAG
    // ENFORCE RULE: SECOND TOKEN MUST BE TAG NAME
    // ENFORCE RULE: THIRD TOKEN MUST BE CLOSE_TAG | PROPERTY
    if (token_vector->size > 2 && 
        *((enum Token*) Vector_Get(token_vector, 0)) == OPEN_TAG && 
        *((enum Token*) Vector_Get(token_vector, 1)) == WINDOW_TAG) return 0; // Success
    else {
        printf("%s", "[Generate_Tree] Error! XML tree has no root. XML documents must begin with a <window> tag.");
        return -1;
    }
}

int AssertNewTagGrammar(struct Vector* token_vector, int i)
{
    // ENFORCE RULE: NEXT TOKEN SHOULD BE TAG NAME 
    if (i < token_vector->size - 2 && TokenToTag(*((enum Token*) Vector_Get(token_vector, i+1))) != NAT)
    {
        enum Token third_token = *((enum Token*) Vector_Get(token_vector, i+2));
        if (third_token == CLOSE_TAG || third_token == CLOSE_END_TAG || isPropertyToken(third_token)) return 0; // Success
    }
    return -1; // Failure
}

int AssertTagCloseStartGrammar(struct Vector* token_vector, int i, enum Tag openTag)
{
    // ENFORCE RULE: NEXT TOKEN SHOULD BE TAG AND MUST MATCH OPENING TAG
    // ENDORCE RULE: THIRD TOKEN MUST BE A TAG END
    if (i < token_vector->size - 2 && 
        TokenToTag(*((enum Token*) Vector_Get(token_vector, i+1))) == openTag && 
        *((enum Token*) Vector_Get(token_vector, i+2)) == CLOSE_TAG) return 0; // Success
    else {
        printf("%s", "[Generate Tree] Error! Closing tag does not match.");
        printf("%s %d %s %d", "close tag:", TokenToTag(*((enum Token*) Vector_Get(token_vector, i+1))), "open tag:", openTag);
    }
    return -1; // Failure
}

int generate_tree(char* src_buffer, uint32_t src_length, struct Vector* token_vector, struct Vector* ptext_ref_vector)
{
    // Create the tree data structure -> reserve 64 layers, 100 nodes per layer = 384KB
    const int max_stack_depth = 32;
    struct Vector tree_stack[max_stack_depth];
    Vector_Reserve(&tree_stack[0], sizeof(struct Node), 1); // 1 root element
    for (int i=1; i<max_stack_depth; i++) {
        Vector_Reserve(&tree_stack[i], sizeof(struct Node), 100);
    }

    // Enforce root grammar
    if (AssertRootGrammar(token_vector) != 0) {
        return -1; // Failure 
    }

    // Iterate over all tokens
    int i = 0;
    int currentParentLayer = -1;
    while (i < token_vector->size)
    {
        const enum Token token = *((enum Token*) Vector_Get(token_vector, i));

        // New tag -> Add a new node
        if (token == OPEN_TAG)
        {
            if (AssertNewTagGrammar(token_vector, i) == 0)
            {
                // Enforce max tree depth
                if (currentParentLayer+1 == max_stack_depth)
                {
                    printf("%s", "[Generate Tree] Error! Exceeded max tree depth of 64");
                    return -1; // Failure
                }


                // Create a new node
                struct Node newNode;
                newNode.tag = TokenToTag(*((enum Token*) Vector_Get(token_vector, i+1)));
                newNode.child_count = 0;
                newNode.first_child_index = -1;
                newNode.parent_index = tree_stack[currentParentLayer].size - 1;
                Vector_Push(&tree_stack[currentParentLayer+1], &newNode);


                if (currentParentLayer != -1) // Only equals -1 for the root window node (optimisation would be to remvoe this check and append root node at beggining of function)
                {
                    // Inform parent that parent has new child
                    struct Node* parentNode = (struct Node*) Vector_Get(&tree_stack[currentParentLayer], newNode.parent_index);
                    if (parentNode->child_count == 0)
                    {
                        parentNode->first_child_index = tree_stack[currentParentLayer+1].size - 1;
                    }
                    parentNode->child_count += 1;
                }

                

                // Move one layer deeper
                currentParentLayer++;

                // Increment token
                i+=2;
                continue;
            }
            else
            {
                return -1;
            }
        }

        // Open end tag -> tag closes -> move up one layer
        if (token == OPEN_END_TAG)
        {
            if (currentParentLayer < 0) break;
        
            enum Tag openTag = ((struct Node*) Vector_Get(&tree_stack[currentParentLayer], tree_stack[currentParentLayer].size - 1))->tag;
           

            if (AssertTagCloseStartGrammar(token_vector, i, openTag) != 0)
            {
                return -1; // Failure
            }

            // Move one layer higher
            currentParentLayer--;

            // Increment token
            i+=3;
            continue;
        }

        // Close end tag -> tag self closes -> move up one layer
        if (token == CLOSE_END_TAG)
        {
            // Move one layer higher
            currentParentLayer--;

            // Increment token
            i+=1;
            continue;
        }
        
        i+= 1;
    }

    return 0; // Success
}

int parse(char* filepath)
{
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Cannot open file '%s': %s\n", filepath, strerror(errno));
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long file_size_long = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size_long > UINT32_MAX) {
        printf("%s", "Src file is too large! It must be < 4 294 967 295 Bytes");
        return -1;
    }

    // Create a src buffer
    uint32_t src_length = (uint32_t)file_size_long;
    char* src_buffer = malloc(src_length);
    fread(src_buffer, 1, src_length, f);
    fclose(f);

    // Init token vector and reserve ~1MB
    struct Vector token_vector;
    Vector_Reserve(&token_vector, sizeof(enum Token), 250000);

    // Init property text reference vector and reserve ~900KB
    struct Vector ptext_ref_vector;
    Vector_Reserve(&ptext_ref_vector, sizeof(struct PTextRef), 100000);

    // Tokenise the file source
    tokenise(src_buffer, src_length, &token_vector, &ptext_ref_vector);

    // Generate UI tree
    generate_tree(src_buffer, src_length, &token_vector, &ptext_ref_vector);


    // Free token vector memory
    Vector_Free(&token_vector);
    return 0;
}

int main()
{
    printf("Node size: %zu\n", sizeof(struct Node));
    timer_start();
    start_measurement();
    parse("tiny.xml");
    end_measurement();
    timer_stop();
}