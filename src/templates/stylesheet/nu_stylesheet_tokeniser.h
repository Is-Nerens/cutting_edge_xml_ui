#pragma once
#include <datastructures/utf8_parser_word.h>
#include "../nu_token_array.h"
#include "nu_stylesheet_tokens.h"

enum CSS_Tokenise_Ctx
{
    CSS_TOKENISE_CTX_GLOBAL_SPACE,
    CSS_TOKENISE_CTX_GLOBAL_COMMENT_SPACE,
    CSS_TOKENISE_CTX_SELECTOR_SPACE,
    CSS_TOKENISE_CTX_SELECTOR_COMMENT_SPACE,
    CSS_TOKENISE_CTX_PROPERTY_VALUE_SPACE,
    CSS_TOKENISE_CTX_PROPERTY_STRING_VALUE_SPACE,
    CSS_TOKENISE_CTX_PROPERTY_VALUE_STRING_SPACE_ESCAPE_CHAR,
    CSS_TOKENISE_CTX_VARIABLE_NAME_DEFINE_SPACE,
    CSS_TOKENISE_CTX_CLASS_SELECTOR_NAME_SPACE,
    CSS_TOKENISE_CTX_ID_SELECTOR_NAME_SPACE,
    CSS_TOKENISE_CTX_PSEUDO_NAME_SPACE,
    CSS_TOKENISE_CTX_FONT_CREATION_NAME_SPACE,
    CSS_TOKENISE_CTX_SCREEN_SELECTOR_NAME_SPACE, // Eg. @screen > 2000 (after @screen and before the {)
};

static void NU_Style_Tokenise(String src, TokenArray* tokens, Array* textRefs)
{
    ParserWord word;
    ParserWord_Init(&word);

    // Context
    u8 ctx = CSS_TOKENISE_CTX_GLOBAL_SPACE;

    // Iterate over src file
    u32 srcLen = StringLen(src);
    int i = 0;
    while (i < srcLen)
    {
        u32 c = NextUTF8Codepoint(src, &i);

        // Globalspace comment begins
        int peekI = i;
        if (ctx == CSS_TOKENISE_CTX_GLOBAL_SPACE && i < srcLen - 1 && c == '/' && NextUTF8Codepoint(src, &peekI) == '*') {
            i=peekI; ctx = CSS_TOKENISE_CTX_GLOBAL_COMMENT_SPACE; continue; // ^
        }

        // Globalspace comment ends
        if (ctx == CSS_TOKENISE_CTX_GLOBAL_COMMENT_SPACE && i < srcLen - 1 && c == '*' && NextUTF8Codepoint(src, &peekI) == '/') {
            i=peekI; ctx = CSS_TOKENISE_CTX_GLOBAL_SPACE; continue; // ^
        }

        // Selectorspace comment begins
        if (ctx == CSS_TOKENISE_CTX_SELECTOR_SPACE && i < srcLen - 1 && c == '/' && NextUTF8Codepoint(src, &peekI) == '*') {
            i=peekI; ctx = CSS_TOKENISE_CTX_SELECTOR_COMMENT_SPACE; continue; // ^
        }

        // Selectorspace comment ends
        if (ctx == CSS_TOKENISE_CTX_SELECTOR_COMMENT_SPACE && i < srcLen - 1 && c == '*' && NextUTF8Codepoint(src, &peekI) == '/') {
            i=peekI; ctx = CSS_TOKENISE_CTX_SELECTOR_SPACE; continue; // ^
        }

        // Variable name definition space
        if (ctx == CSS_TOKENISE_CTX_SELECTOR_SPACE && i < srcLen - 1 && c == '-' && NextUTF8Codepoint(src, &peekI) == '-') {
            ParserWord_Append(&word, '-');
            ParserWord_Append(&word, '-');
            i=peekI; ctx = CSS_TOKENISE_CTX_VARIABLE_NAME_DEFINE_SPACE; continue; // ^
        }


        // In comment -> skip rest
        if (ctx == CSS_TOKENISE_CTX_GLOBAL_COMMENT_SPACE || ctx == CSS_TOKENISE_CTX_SELECTOR_COMMENT_SPACE) {
            continue; // ^
        }

        // Enter class selector name space
        if (ctx == CSS_TOKENISE_CTX_GLOBAL_SPACE && c == '.') {
            ParserWord_Clear(&word); ctx = CSS_TOKENISE_CTX_CLASS_SELECTOR_NAME_SPACE; continue; // ^
        }

        // Enter id selector name space
        if (ctx == CSS_TOKENISE_CTX_GLOBAL_SPACE && c == '#') {
            ParserWord_Clear(&word); ctx = CSS_TOKENISE_CTX_ID_SELECTOR_NAME_SPACE; continue; // ^
        }

        // Property value word completed
        if (ctx == CSS_TOKENISE_CTX_PROPERTY_VALUE_SPACE && c == ';') {

            // Add property text reference
            if (word.length > 0) {
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);

                // Determine if is variable property name or hard-coded property value
                if (word.length > 2 && word.buffer[0] == '-' && word.buffer[1] == '-') {
                    TokenArray_Add(tokens, STYLE_VARIABLE_PROPERTY_VALUE);
                }
                else {
                    TokenArray_Add(tokens, STYLE_PROPERTY_VALUE);
                }
                ParserWord_Clear(&word);
            }
            ctx = CSS_TOKENISE_CTX_SELECTOR_SPACE; continue; // ^
        }

        // Property value quotes string started
        if (ctx == CSS_TOKENISE_CTX_PROPERTY_VALUE_SPACE && c == '"') {
            ParserWord_Clear(&word); ctx = CSS_TOKENISE_CTX_PROPERTY_STRING_VALUE_SPACE; continue; // ^
        }
        if (ctx == CSS_TOKENISE_CTX_PROPERTY_STRING_VALUE_SPACE && c =='\\') {
            ctx = CSS_TOKENISE_CTX_PROPERTY_VALUE_STRING_SPACE_ESCAPE_CHAR; continue; // ^
        }
        if (ctx == CSS_TOKENISE_CTX_PROPERTY_VALUE_STRING_SPACE_ESCAPE_CHAR) {
            ctx = CSS_TOKENISE_CTX_PROPERTY_STRING_VALUE_SPACE; continue; // ^
        }
        // Property value quotes string completed
        if (ctx == CSS_TOKENISE_CTX_PROPERTY_STRING_VALUE_SPACE && c == '"') {
            // Add property text reference
            if (word.length > 0) {
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);

                // Determine if is variable property name or hard-coded property value
                if (word.length > 2 && word.buffer[0] == '-' && word.buffer[1] == '-') {
                    TokenArray_Add(tokens, STYLE_VARIABLE_PROPERTY_VALUE);
                }
                else {
                    TokenArray_Add(tokens, STYLE_PROPERTY_VALUE);
                }
                ParserWord_Clear(&word);
            }
            ctx = CSS_TOKENISE_CTX_PROPERTY_VALUE_SPACE; continue; // ^
        }

        // Enter selectorspace
        if (c == '{')
        {
            // Tag selector word completed
            if (ctx == CSS_TOKENISE_CTX_GLOBAL_SPACE && word.length > 0) {
                TokenArray_Add(tokens, NU_Word_To_Tag_Selector_Token(word.buffer, word.length));
                ParserWord_Clear(&word);
            }

            // Class selector word completed
            else if (ctx == CSS_TOKENISE_CTX_CLASS_SELECTOR_NAME_SPACE && word.length > 0) {
                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_CLASS_SELECTOR);
                ParserWord_Clear(&word);
            }

            // Id selector word completed
            else if (ctx == CSS_TOKENISE_CTX_ID_SELECTOR_NAME_SPACE && word.length > 0) {
                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_ID_SELECTOR);
                ParserWord_Clear(&word);
            }

            // Font name word completed
            else if (ctx == CSS_TOKENISE_CTX_FONT_CREATION_NAME_SPACE && word.length > 0) {
                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_FONT_NAME);
                ParserWord_Clear(&word);
            }

            // Add open brace token
            TokenArray_Add(tokens, STYLE_SELECTOR_OPEN_BRACE);
            ParserWord_Clear(&word); ctx = CSS_TOKENISE_CTX_SELECTOR_SPACE; continue; // ^
        }

        // Exiting selectorspace
        if (c == '}')
        {
            // If word is present -> word completed (also an error)
            if (word.length > 0) {
                TokenArray_Add(tokens, NU_Word_To_Style_Token(word.buffer, word.length));
                ParserWord_Clear(&word);
            }

            TokenArray_Add(tokens, STYLE_SELECTOR_CLOSE_BRACE);
            ctx = CSS_TOKENISE_CTX_GLOBAL_SPACE; continue; // ^
        }

        // Encountered separation character -> word is completed
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == ':')
        {
            // Any selector word completed
            if (ctx == CSS_TOKENISE_CTX_GLOBAL_SPACE && word.length > 0) {
                enum NU_Style_Token token = NU_Word_To_Any_Selector_Token(word.buffer, word.length);
                TokenArray_Add(tokens, token);
                ParserWord_Clear(&word);

                if (token == STYLE_FONT_CREATION_SELECTOR) { // Special selector context
                    ctx = CSS_TOKENISE_CTX_FONT_CREATION_NAME_SPACE;
                }
                else if (token == STYLE_SCREEN_SELECTOR) {
                    ctx = CSS_TOKENISE_CTX_SCREEN_SELECTOR_NAME_SPACE;
                }
            }

            // Class selector word completed
            else if (ctx == CSS_TOKENISE_CTX_CLASS_SELECTOR_NAME_SPACE && word.length > 0) {
                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_CLASS_SELECTOR);
                ctx = CSS_TOKENISE_CTX_GLOBAL_SPACE;
            }

            // Id selector word completed
            else if (ctx == CSS_TOKENISE_CTX_ID_SELECTOR_NAME_SPACE && word.length > 0) {
                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_ID_SELECTOR);
                ctx = CSS_TOKENISE_CTX_GLOBAL_SPACE;
            }

            // Property identifier word completed
            else if (ctx == CSS_TOKENISE_CTX_SELECTOR_SPACE && word.length > 0) {
                TokenArray_Add(tokens, NU_Word_To_Style_Property_Token(word.buffer, word.length));
            }

            // Variable name definition word completed
            else if (ctx == CSS_TOKENISE_CTX_VARIABLE_NAME_DEFINE_SPACE && word.length > 0) {
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_VARIABLE_NAME);
                ctx = CSS_TOKENISE_CTX_SELECTOR_SPACE;
            }

            // Property value word completed
            else if (ctx == CSS_TOKENISE_CTX_PROPERTY_VALUE_SPACE && word.length > 0) {
                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);

                // Determine if is variable property value or normal property value
                if (word.length > 2 && word.buffer[0] == '-' && word.buffer[1] == '-') {
                    TokenArray_Add(tokens, STYLE_VARIABLE_PROPERTY_VALUE);
                }
                else {
                    TokenArray_Add(tokens, STYLE_PROPERTY_VALUE);
                }
                ctx = CSS_TOKENISE_CTX_SELECTOR_SPACE;
            }

            // Pseudo class word completed
            else if (ctx == CSS_TOKENISE_CTX_PSEUDO_NAME_SPACE && word.length > 0) {
                TokenArray_Add(tokens, NU_Word_To_Pseudo_Token(word.buffer, word.length));
                ctx = CSS_TOKENISE_CTX_GLOBAL_SPACE;
            }

            // Special font name word completed
            else if (ctx == CSS_TOKENISE_CTX_FONT_CREATION_NAME_SPACE && word.length > 0) {

                // Add text reference
                StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                Array_Push(textRefs, &ref);
                TokenArray_Add(tokens, STYLE_FONT_NAME);
            }

            else if (ctx == CSS_TOKENISE_CTX_SCREEN_SELECTOR_NAME_SPACE && word.length > 0) {

                if (word.length == 1 && word.buffer[0] == '>') {
                    TokenArray_Add(tokens, STYLE_GREATER);
                }
                else if (word.length == 1 && word.buffer[0] == '<') {
                    TokenArray_Add(tokens, STYLE_LESS);
                }
                if (word.length == 2 && word.buffer[0] == '>' && word.buffer[1] == '=') {
                    TokenArray_Add(tokens, STYLE_GREATER_EQUAL);
                }
                else if (word.length == 2 && word.buffer[0] == '<'  && word.buffer[1] == '=') {
                    TokenArray_Add(tokens, STYLE_LESS_EQUAL);
                }
                else {
                    // Add text reference
                    StyleTextRef ref = { tokens->size, i - word.length - 1, word.length };
                    StringCstr(src)[ref.src_index + ref.char_count] = '\0';
                    Array_Push(textRefs, &ref);
                    TokenArray_Add(tokens, STYLE_PROPERTY_VALUE);
                }
            }

            if (c == ',') {
                TokenArray_Add(tokens, STYLE_SELECTOR_COMMA);
            }

            if (c == ':') {
                // Style property assignment token
                if (ctx == CSS_TOKENISE_CTX_SELECTOR_SPACE) {
                    TokenArray_Add(tokens, STYLE_PROPERTY_ASSIGNMENT);
                    ctx = CSS_TOKENISE_CTX_PROPERTY_VALUE_SPACE;
                }
                // Pseudo class assignment token
                else if (ctx == CSS_TOKENISE_CTX_GLOBAL_SPACE || ctx == CSS_TOKENISE_CTX_CLASS_SELECTOR_NAME_SPACE || ctx == CSS_TOKENISE_CTX_ID_SELECTOR_NAME_SPACE ) {
                    TokenArray_Add(tokens, STYLE_PSEUDO_COLON);
                    ctx = CSS_TOKENISE_CTX_PSEUDO_NAME_SPACE;
                }
            }

            ParserWord_Clear(&word);
            continue; // ^
        }

        // Add char to word
        ParserWord_Append(&word, c);
    }
}
