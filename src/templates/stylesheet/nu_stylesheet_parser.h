#pragma once
#include "nu_stylesheet_grammar_assertions.h"
#include "../nu_token_array.h"
#include "nu_stylesheet_structs.h"
#include "nu_stylesheet_tokens.h"

void Stylesheet_Overwrite_Style_Item(Stylesheet_Item* item, Stylesheet_Item* overwriter)
{
    item->propertyFlags |= overwriter->propertyFlags;

    // Branchless layoutFlags update
    item->layoutFlags = (item->layoutFlags & ~LAYOUT_VERTICAL)                 | ((overwriter->layoutFlags & LAYOUT_VERTICAL)                 * !!(overwriter->propertyFlags & PROPERTY_FLAG_LAYOUT_VERTICAL));
    item->layoutFlags = (item->layoutFlags & ~(GROW_HORIZONTAL|GROW_VERTICAL)) | ((overwriter->layoutFlags & (GROW_HORIZONTAL|GROW_VERTICAL)) * !!(overwriter->propertyFlags & PROPERTY_FLAG_GROW));
    item->layoutFlags = (item->layoutFlags & ~OVERFLOW_VERTICAL_SCROLL)        | ((overwriter->layoutFlags & OVERFLOW_VERTICAL_SCROLL)        * !!(overwriter->propertyFlags & PROPERTY_FLAG_VERTICAL_SCROLL));
    item->layoutFlags = (item->layoutFlags & ~OVERFLOW_HORIZONTAL_SCROLL)      | ((overwriter->layoutFlags & OVERFLOW_HORIZONTAL_SCROLL)      * !!(overwriter->propertyFlags & PROPERTY_FLAG_HORIZONTAL_SCROLL));
    item->layoutFlags = (item->layoutFlags & ~POSITION_ABSOLUTE)               | ((overwriter->layoutFlags & POSITION_ABSOLUTE)               * !!(overwriter->propertyFlags & PROPERTY_FLAG_POSITION_ABSOLUTE));
    item->layoutFlags = (item->layoutFlags & ~HIDDEN)                          | ((overwriter->layoutFlags & HIDDEN)                          * !!(overwriter->propertyFlags & PROPERTY_FLAG_HIDDEN));
    item->layoutFlags = (item->layoutFlags & ~IGNORE_MOUSE)                    | ((overwriter->layoutFlags & IGNORE_MOUSE)                    * !!(overwriter->propertyFlags & PROPERTY_FLAG_IGNORE_MOUSE));
    item->layoutFlags = (item->layoutFlags & ~HIDE_BACKGROUND)                 | ((overwriter->layoutFlags & HIDE_BACKGROUND)                 * !!(overwriter->propertyFlags & PROPERTY_FLAG_HIDE_BACKGROUND));

    // Overwrite gap and size fields (branchless)
    item->gap        = item->gap        * !(overwriter->propertyFlags & PROPERTY_FLAG_GAP)              + overwriter->gap        * !!(overwriter->propertyFlags & PROPERTY_FLAG_GAP);
    item->prefWidth  = item->prefWidth  * !(overwriter->propertyFlags & PROPERTY_FLAG_PREFERRED_WIDTH)  + overwriter->prefWidth  * !!(overwriter->propertyFlags & PROPERTY_FLAG_PREFERRED_WIDTH);
    item->minWidth   = item->minWidth   * !(overwriter->propertyFlags & PROPERTY_FLAG_MIN_WIDTH)        + overwriter->minWidth   * !!(overwriter->propertyFlags & PROPERTY_FLAG_MIN_WIDTH);
    item->maxWidth   = item->maxWidth   * !(overwriter->propertyFlags & PROPERTY_FLAG_MAX_WIDTH)        + overwriter->maxWidth   * !!(overwriter->propertyFlags & PROPERTY_FLAG_MAX_WIDTH);
    item->prefHeight = item->prefHeight * !(overwriter->propertyFlags & PROPERTY_FLAG_PREFERRED_HEIGHT) + overwriter->prefHeight * !!(overwriter->propertyFlags & PROPERTY_FLAG_PREFERRED_HEIGHT);
    item->minHeight  = item->minHeight  * !(overwriter->propertyFlags & PROPERTY_FLAG_MIN_HEIGHT)       + overwriter->minHeight  * !!(overwriter->propertyFlags & PROPERTY_FLAG_MIN_HEIGHT);
    item->maxHeight  = item->maxHeight  * !(overwriter->propertyFlags & PROPERTY_FLAG_MAX_HEIGHT)       + overwriter->maxHeight  * !!(overwriter->propertyFlags & PROPERTY_FLAG_MAX_HEIGHT);

    // Overwrite alignments (branchless)
    item->horizontalAlignment     = item->horizontalAlignment     * !(overwriter->propertyFlags & PROPERTY_FLAG_ALIGN_H)      + overwriter->horizontalAlignment     * !!(overwriter->propertyFlags & PROPERTY_FLAG_ALIGN_H);
    item->verticalAlignment       = item->verticalAlignment       * !(overwriter->propertyFlags & PROPERTY_FLAG_ALIGN_V)      + overwriter->verticalAlignment       * !!(overwriter->propertyFlags & PROPERTY_FLAG_ALIGN_V);
    item->horizontalTextAlignment = item->horizontalTextAlignment * !(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_ALIGN_H) + overwriter->horizontalTextAlignment * !!(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_ALIGN_H);
    item->verticalTextAlignment   = item->verticalTextAlignment   * !(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_ALIGN_V) + overwriter->verticalTextAlignment   * !!(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_ALIGN_V);

    // Overwrite absolute distances (branchless)
    item->left   = item->left   * !(overwriter->propertyFlags & PROPERTY_FLAG_LEFT)   + overwriter->left   * !!(overwriter->propertyFlags & PROPERTY_FLAG_LEFT);
    item->right  = item->right  * !(overwriter->propertyFlags & PROPERTY_FLAG_RIGHT)  + overwriter->right  * !!(overwriter->propertyFlags & PROPERTY_FLAG_RIGHT);
    item->top    = item->top    * !(overwriter->propertyFlags & PROPERTY_FLAG_TOP)    + overwriter->top    * !!(overwriter->propertyFlags & PROPERTY_FLAG_TOP);
    item->bottom = item->bottom * !(overwriter->propertyFlags & PROPERTY_FLAG_BOTTOM) + overwriter->bottom * !!(overwriter->propertyFlags & PROPERTY_FLAG_BOTTOM);

    // Branchless RGB overwrite
    item->backgroundR = item->backgroundR * !(overwriter->propertyFlags & PROPERTY_FLAG_BACKGROUND)    + overwriter->backgroundR * !!(overwriter->propertyFlags & PROPERTY_FLAG_BACKGROUND);
    item->backgroundG = item->backgroundG * !(overwriter->propertyFlags & PROPERTY_FLAG_BACKGROUND)    + overwriter->backgroundG * !!(overwriter->propertyFlags & PROPERTY_FLAG_BACKGROUND);
    item->backgroundB = item->backgroundB * !(overwriter->propertyFlags & PROPERTY_FLAG_BACKGROUND)    + overwriter->backgroundB * !!(overwriter->propertyFlags & PROPERTY_FLAG_BACKGROUND);
    item->borderR     = item->borderR     * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_COLOUR) + overwriter->borderR     * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_COLOUR);
    item->borderG     = item->borderG     * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_COLOUR) + overwriter->borderG     * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_COLOUR);
    item->borderB     = item->borderB     * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_COLOUR) + overwriter->borderB     * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_COLOUR);
    item->textR       = item->textR       * !(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_COLOUR)   + overwriter->textR       * !!(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_COLOUR);
    item->textG       = item->textG       * !(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_COLOUR)   + overwriter->textG       * !!(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_COLOUR);
    item->textB       = item->textB       * !(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_COLOUR)   + overwriter->textB       * !!(overwriter->propertyFlags & PROPERTY_FLAG_TEXT_COLOUR);

    // Overwrite border widths (branchless)
    item->borderTop    = item->borderTop    * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_TOP)    + overwriter->borderTop    * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_TOP);
    item->borderBottom = item->borderBottom * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_BOTTOM) + overwriter->borderBottom * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_BOTTOM);
    item->borderLeft   = item->borderLeft   * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_LEFT)   + overwriter->borderLeft   * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_LEFT);
    item->borderRight  = item->borderRight  * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RIGHT)  + overwriter->borderRight  * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RIGHT);

    // Overwrite border radii (branchless)
    item->borderRadiusTl = item->borderRadiusTl * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_TL) + overwriter->borderRadiusTl * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_TL);
    item->borderRadiusTr = item->borderRadiusTr * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_TR) + overwriter->borderRadiusTr * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_TR);
    item->borderRadiusBl = item->borderRadiusBl * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_BL) + overwriter->borderRadiusBl * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_BL);
    item->borderRadiusBr = item->borderRadiusBr * !(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_BR) + overwriter->borderRadiusBr * !!(overwriter->propertyFlags & PROPERTY_FLAG_BORDER_RADIUS_BR);

    // Overwrite padding (branchless)
    item->padTop    = item->padTop    * !(overwriter->propertyFlags & PROPERTY_FLAG_PAD_TOP)    + overwriter->padTop    * !!(overwriter->propertyFlags & PROPERTY_FLAG_PAD_TOP);
    item->padBottom = item->padBottom * !(overwriter->propertyFlags & PROPERTY_FLAG_PAD_BOTTOM) + overwriter->padBottom * !!(overwriter->propertyFlags & PROPERTY_FLAG_PAD_BOTTOM);
    item->padLeft   = item->padLeft   * !(overwriter->propertyFlags & PROPERTY_FLAG_PAD_LEFT)   + overwriter->padLeft   * !!(overwriter->propertyFlags & PROPERTY_FLAG_PAD_LEFT);
    item->padRight  = item->padRight  * !(overwriter->propertyFlags & PROPERTY_FLAG_PAD_RIGHT)  + overwriter->padRight  * !!(overwriter->propertyFlags & PROPERTY_FLAG_PAD_RIGHT);

    // Overwrite image and input type (branchless)
    item->imageHandle = item->imageHandle * !(overwriter->propertyFlags & PROPERTY_FLAG_IMAGE)      + overwriter->imageHandle * !!(overwriter->propertyFlags & PROPERTY_FLAG_IMAGE);
    item->inputType   = item->inputType   * !(overwriter->propertyFlags & PROPERTY_FLAG_INPUT_TYPE) + overwriter->inputType   * !!(overwriter->propertyFlags & PROPERTY_FLAG_INPUT_TYPE);

    // Overwrite font Id
    item->fontId = overwriter->fontId;
}

void Stylesheet_Parse_Property(Stylesheet* ss, const enum NU_Style_Token token, Stylesheet_Item* item, const char* text, int textLen, ImageResourceLoader* imageResourceLoader)
{
    switch (token)
    {
        // Set layout direction
        case STYLE_LAYOUT_DIRECTION_PROPERTY:
            switch(text[0])
            {
                case 'v':
                    item->layoutFlags |= LAYOUT_VERTICAL;
                    item->propertyFlags |= PROPERTY_FLAG_LAYOUT_VERTICAL; break;
                    break;
                case 'h':
                    item->propertyFlags |= PROPERTY_FLAG_LAYOUT_VERTICAL; break;
            }
            break;

        // Set growth
        case STYLE_GROW_PROPERTY:
            switch(text[0])
            {
                case 'v': item->layoutFlags |= GROW_VERTICAL; break;
                case 'h': item->layoutFlags |= GROW_HORIZONTAL; break;
                case 'b': item->layoutFlags |= (GROW_HORIZONTAL | GROW_VERTICAL); break;
            }
            item->propertyFlags |= PROPERTY_FLAG_GROW;
            break;

        // Set overflow behaviour
        case STYLE_OVERFLOW_V_PROPERTY:
            switch(text[0])
            {
                case 's':
                    item->layoutFlags |= OVERFLOW_VERTICAL_SCROLL;
                    item->propertyFlags |= PROPERTY_FLAG_VERTICAL_SCROLL; break;
                case 'h':
                    item->propertyFlags |= PROPERTY_FLAG_VERTICAL_SCROLL; break;
            }
            break;

        case STYLE_OVERFLOW_H_PROPERTY:
            switch(text[0])
            {
                case 's':
                    item->layoutFlags |= OVERFLOW_HORIZONTAL_SCROLL;
                    item->propertyFlags |= PROPERTY_FLAG_HORIZONTAL_SCROLL; break;
                case 'h':
                    item->propertyFlags |= PROPERTY_FLAG_HORIZONTAL_SCROLL; break;
            }
            break;

        // Relative/Absolute positioning
        case STYLE_POSITION_PROPERTY:
            if (strcmp(text, "absolute") == 0) {
                item->layoutFlags |= POSITION_ABSOLUTE;
                item->propertyFlags |= PROPERTY_FLAG_POSITION_ABSOLUTE;
            } else if (strcmp(text, "relative") == 0) {
                item->propertyFlags |= PROPERTY_FLAG_POSITION_ABSOLUTE;
            }
            break;

        // Hide/show
        case STYLE_HIDE_PROPERTY:
            if (strcmp(text, "true") == 0) {
                item->layoutFlags |= HIDDEN;
                item->propertyFlags |= PROPERTY_FLAG_HIDDEN;
            }
            else if (strcmp(text, "false") == 0) {
                item->propertyFlags |= PROPERTY_FLAG_HIDDEN;
            }
            break;

        // Ignore mouse
        case STYLE_IGNORE_MOUSE_PROPERTY:
            if (strcmp(text, "true") == 0) {
                item->layoutFlags |= IGNORE_MOUSE;
                item->propertyFlags |= PROPERTY_FLAG_IGNORE_MOUSE;
            }
            else if (strcmp(text, "false") == 0) {
                item->propertyFlags |= PROPERTY_FLAG_IGNORE_MOUSE;
            }
            break;

        // Set gap
        case STYLE_GAP_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_GAP * String_To_u8(&item->gap, text));
            break;

        // Set preferred width
        case STYLE_WIDTH_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_PREFERRED_WIDTH * String_To_Uint16(&item->prefWidth, text));
            break;

        // Set min width
        case STYLE_MIN_WIDTH_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_MIN_WIDTH * String_To_Uint16(&item->minWidth, text));
            break;

        // Set max width
        case STYLE_MAX_WIDTH_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_MAX_WIDTH * String_To_Uint16(&item->maxWidth, text));
            break;

        // Set preferred height
        case STYLE_HEIGHT_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_PREFERRED_HEIGHT * String_To_Uint16(&item->prefHeight, text));
            break;

        // Set min height
        case STYLE_MIN_HEIGHT_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_MIN_HEIGHT * String_To_Uint16(&item->minHeight, text));
            break;

        // Set max height
        case STYLE_MAX_HEIGHT_PROPERTY:
            item->propertyFlags |= (PROPERTY_FLAG_MAX_HEIGHT * String_To_Uint16(&item->maxHeight, text));
            break;

        // Set horizontal alignment
        case STYLE_ALIGN_H_PROPERTY:
            if (strcmp(text, "left") == 0) {
                item->horizontalAlignment = 0;
                item->propertyFlags |= PROPERTY_FLAG_ALIGN_H;
            }
            else if (strcmp(text, "center") == 0) {
                item->horizontalAlignment = 1;
                item->propertyFlags |= PROPERTY_FLAG_ALIGN_H;
            }
            else if (strcmp(text, "right") == 0) {
                item->horizontalAlignment = 2;
                item->propertyFlags |= PROPERTY_FLAG_ALIGN_H;
            }
            break;

        // Set vertical alignment
        case STYLE_ALIGN_V_PROPERTY:
            if (strcmp(text, "top") == 0) {
                item->verticalAlignment = 0;
                item->propertyFlags |= PROPERTY_FLAG_ALIGN_V;
            }
            else if (strcmp(text, "center") == 0) {
                item->verticalAlignment = 1;
                item->propertyFlags |= PROPERTY_FLAG_ALIGN_V;
            }
            else if (strcmp(text, "bottom") == 0) {
                item->verticalAlignment = 2;
                item->propertyFlags |= PROPERTY_FLAG_ALIGN_V;
            }
            break;

        // Set horizontal text alignment
        case STYLE_TEXT_ALIGN_H_PROPERTY:
            if (strcmp(text, "left") == 0) {
                item->horizontalTextAlignment = 0;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_H;
            }
            else if (strcmp(text, "center") == 0) {
                item->horizontalTextAlignment = 1;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_H;
            }
            else if (strcmp(text, "right") == 0) {
                item->horizontalTextAlignment = 2;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_H;
            }
            break;

        // Set vertical text alignment
        case STYLE_TEXT_ALIGN_V_PROPERTY:
            if (strcmp(text, "top") == 0) {
                item->verticalTextAlignment = 0;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_V;
            }
            else if (strcmp(text, "center") == 0) {
                item->verticalTextAlignment = 1;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_V;
            }
            else if (strcmp(text, "bottom") == 0) {
                item->verticalTextAlignment = 2;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_V;
            }
            break;

        // Set absolute position properties
        case STYLE_LEFT_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_LEFT * String_To_Int16(&item->left, text);
            break;

        case STYLE_RIGHT_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_RIGHT * String_To_Int16(&item->right, text);
            break;

        case STYLE_TOP_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_TOP * String_To_Int16(&item->top, text);
            break;

        case STYLE_BOTTOM_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BOTTOM * String_To_Int16(&item->bottom, text);
            break;

        // Set background colour
        case STYLE_BACKGROUND_COLOUR_PROPERTY: {
            struct RGB rgb;
            if (Parse_Hexcode(text, textLen, &rgb)) {
                item->backgroundR = rgb.r;
                item->backgroundG = rgb.g;
                item->backgroundB = rgb.b;
                item->propertyFlags |= PROPERTY_FLAG_BACKGROUND;
            } else if (strcmp(text, "none") == 0) {
                item->propertyFlags |= PROPERTY_FLAG_HIDE_BACKGROUND;
                item->layoutFlags |= HIDE_BACKGROUND;
            }
            break;
        }

        // Set border colour
        case STYLE_BORDER_COLOUR_PROPERTY: {
            struct RGB rgb;
            if (Parse_Hexcode(text, textLen, &rgb)) {
                item->borderR = rgb.r;
                item->borderG = rgb.g;
                item->borderB = rgb.b;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_COLOUR;
            }
            break;
        }

        // Set text colour
        case STYLE_TEXT_COLOUR_PROPERTY: {
            struct RGB rgb;
            if (Parse_Hexcode(text, textLen, &rgb)) {
                item->textR = rgb.r;
                item->textG = rgb.g;
                item->textB = rgb.b;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_COLOUR;
            }
            break;
        }

        // Set border width
        case STYLE_BORDER_WIDTH_PROPERTY: {
            u8 border_width;
            if (String_To_u8(&border_width, text)) {
                item->borderTop = border_width;
                item->borderBottom = border_width;
                item->borderLeft = border_width;
                item->borderRight = border_width;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_TOP;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_BOTTOM;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_LEFT;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RIGHT;
            }
            break;
        }

        case STYLE_BORDER_TOP_WIDTH_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_TOP * String_To_u8(&item->borderTop, text);
            break;

        case STYLE_BORDER_BOTTOM_WIDTH_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_BOTTOM * String_To_u8(&item->borderBottom, text);
            break;

        case STYLE_BORDER_LEFT_WIDTH_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_LEFT * String_To_u8(&item->borderLeft, text);
            break;

        case STYLE_BORDER_RIGHT_WIDTH_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_RIGHT * String_To_u8(&item->borderRight, text);
            break;

        // Set border radii
        case STYLE_BORDER_RADIUS_PROPERTY: {
            u8 border_radius;
            if (String_To_u8(&border_radius, text)) {
                item->borderRadiusTl = border_radius;
                item->borderRadiusTr = border_radius;
                item->borderRadiusBl = border_radius;
                item->borderRadiusBr = border_radius;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TL;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TR;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BL;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BR;
            }
            break;
        }

        case STYLE_BORDER_TOP_LEFT_RADIUS_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TL *
                String_To_u8(&item->borderRadiusTl, text);
            break;

        case STYLE_BORDER_TOP_RIGHT_RADIUS_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TR *
                String_To_u8(&item->borderRadiusTr, text);
            break;

        case STYLE_BORDER_BOTTOM_LEFT_RADIUS_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BL *
                String_To_u8(&item->borderRadiusBl, text);
            break;

        case STYLE_BORDER_BOTTOM_RIGHT_RADIUS_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BR *
                String_To_u8(&item->borderRadiusBr, text);
            break;

        // Set padding
        case STYLE_PADDING_PROPERTY: {
            u8 pad;
            if (String_To_u8(&pad, text)) {
                item->padTop = pad;
                item->padBottom = pad;
                item->padLeft = pad;
                item->padRight = pad;
                item->propertyFlags |= PROPERTY_FLAG_PAD_TOP;
                item->propertyFlags |= PROPERTY_FLAG_PAD_BOTTOM;
                item->propertyFlags |= PROPERTY_FLAG_PAD_LEFT;
                item->propertyFlags |= PROPERTY_FLAG_PAD_RIGHT;
            }
            break;
        }

        case STYLE_PADDING_TOP_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_PAD_TOP *
                String_To_u8(&item->padTop, text);
            break;

        case STYLE_PADDING_BOTTOM_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_PAD_BOTTOM *
                String_To_u8(&item->padBottom, text);
            break;

        case STYLE_PADDING_LEFT_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_PAD_LEFT *
                String_To_u8(&item->padLeft, text);
            break;

        case STYLE_PADDING_RIGHT_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_PAD_RIGHT *
                String_To_u8(&item->padRight, text);
            break;

        case STYLE_IMAGE_SOURCE_PROPERTY: {
            int imageHandle = ImageResourceLoader_GetLoadedImageHandle(imageResourceLoader, text);

            // Image not loaded yet
            if (imageHandle == 0) {
                imageHandle = ImageResourceLoader_LoadImage(imageResourceLoader, text);
                item->imageHandle = imageHandle;
                item->propertyFlags |= PROPERTY_FLAG_IMAGE;
            }
            else {
                item->imageHandle = imageHandle;
                item->propertyFlags |= PROPERTY_FLAG_IMAGE;
            }
            break;
        }

        case STYLE_FONT_PROPERTY: {
            void* found_font = LinearStringmap_Get(&ss->fontNameIndexMap, text);
            if (found_font != NULL) {
                item->fontId = *(u8*)found_font;
            }
            break;
        }

        case STYLE_INPUT_TYPE_PROPERTY:
            item->propertyFlags |= PROPERTY_FLAG_INPUT_TYPE;
            if (strcmp(text, "number") == 0) {
                item->inputType = 1;
            } else {
                item->inputType = 0;
            }
            break;

        default:
            break;
    }
}

void Stylesheet_Parse_Variable_Property(Stylesheet* ss, const enum NU_Style_Token token, Stylesheet_Item* item, StylesheetVariable variable, ImageResourceLoader* imageResourceLoader)
{
    switch (token)
    {
        // Set layout direction
        case STYLE_LAYOUT_DIRECTION_PROPERTY:
            switch(variable.type)
            {
                case STYLESHEET_VARIABLE_DTYPE_VERTICAL:
                    item->layoutFlags |= LAYOUT_VERTICAL;
                    item->propertyFlags |= PROPERTY_FLAG_LAYOUT_VERTICAL; break;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_HORIZONTAL:
                    item->propertyFlags |= PROPERTY_FLAG_LAYOUT_VERTICAL; break;
                default:
                    break;
            }
            break;

        // Set growth
        case STYLE_GROW_PROPERTY:
            switch(variable.type)
            {
                case STYLESHEET_VARIABLE_DTYPE_VERTICAL: item->layoutFlags |= GROW_VERTICAL; break;
                case STYLESHEET_VARIABLE_DTYPE_HORIZONTAL: item->layoutFlags |= GROW_HORIZONTAL; break;
                case STYLESHEET_VARIABLE_DTYPE_BOTH: item->layoutFlags |= (GROW_HORIZONTAL | GROW_VERTICAL); break;
                default: break;
            }
            item->propertyFlags |= PROPERTY_FLAG_GROW;
            break;

        // Set overflow behaviour
        case STYLE_OVERFLOW_V_PROPERTY:
            switch(variable.type)
            {
                case STYLESHEET_VARIABLE_DTYPE_SCROLL:
                    item->layoutFlags |= OVERFLOW_VERTICAL_SCROLL;
                    item->propertyFlags |= PROPERTY_FLAG_VERTICAL_SCROLL; break;
                case STYLESHEET_VARIABLE_DTYPE_HORIZONTAL:
                    item->propertyFlags |= PROPERTY_FLAG_VERTICAL_SCROLL; break;
                default:
                    break;
            }
            break;

        case STYLE_OVERFLOW_H_PROPERTY:
            switch(variable.type)
            {
                case STYLESHEET_VARIABLE_DTYPE_SCROLL:
                    item->layoutFlags |= OVERFLOW_HORIZONTAL_SCROLL;
                    item->propertyFlags |= PROPERTY_FLAG_HORIZONTAL_SCROLL; break;
                case STYLESHEET_VARIABLE_DTYPE_HORIZONTAL:
                    item->propertyFlags |= PROPERTY_FLAG_HORIZONTAL_SCROLL; break;
                default:
                    break;
            }
            break;

        // Relative/Absolute positioning
        case STYLE_POSITION_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_ABSOLUTE) {
                item->layoutFlags |= POSITION_ABSOLUTE;
                item->propertyFlags |= PROPERTY_FLAG_POSITION_ABSOLUTE;
            } else if (variable.type == STYLESHEET_VARIABLE_DTYPE_RELATIVE) {
                item->propertyFlags |= PROPERTY_FLAG_POSITION_ABSOLUTE;
            }
            break;

        // Hide/show
        case STYLE_HIDE_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_TRUE) {
                item->layoutFlags |= HIDDEN;
                item->propertyFlags |= PROPERTY_FLAG_HIDDEN;
            }
            else if (variable.type == STYLESHEET_VARIABLE_DTYPE_FALSE) {
                item->propertyFlags |= PROPERTY_FLAG_HIDDEN;
            }
            break;

        // Ignore mouse
        case STYLE_IGNORE_MOUSE_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_TRUE) {
                item->layoutFlags |= IGNORE_MOUSE;
                item->propertyFlags |= PROPERTY_FLAG_IGNORE_MOUSE;
            }
            else if (variable.type == STYLESHEET_VARIABLE_DTYPE_FALSE) {
                item->propertyFlags |= PROPERTY_FLAG_IGNORE_MOUSE;
            }
            break;

        // Set gap
        case STYLE_GAP_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->gap = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_GAP;
            }
            break;

        // Set preferred width
        case STYLE_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->prefWidth = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_PREFERRED_WIDTH;
            }
            break;

        // Set min width
        case STYLE_MIN_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->minWidth = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_MIN_WIDTH;
            }
            break;

        // Set max width
        case STYLE_MAX_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->maxWidth = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_MAX_WIDTH;
            }
            break;

        // Set preferred height
        case STYLE_HEIGHT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->prefHeight = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_PREFERRED_HEIGHT;
            }
            break;

        // Set min height
        case STYLE_MIN_HEIGHT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->minHeight = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_MIN_HEIGHT;
            }
            break;

        // Set max height
        case STYLE_MAX_HEIGHT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->maxHeight = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_MAX_HEIGHT;
            }
            break;

        // Set horizontal alignment
        case STYLE_ALIGN_H_PROPERTY:
            switch(variable.type) {
                case STYLESHEET_VARIABLE_DTYPE_LEFT:
                    item->horizontalAlignment = 0;
                    item->propertyFlags |= PROPERTY_FLAG_ALIGN_H;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_CENTER:
                    item->horizontalAlignment = 1;
                    item->propertyFlags |= PROPERTY_FLAG_ALIGN_H;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_RIGHT:
                    item->horizontalAlignment = 2;
                    item->propertyFlags |= PROPERTY_FLAG_ALIGN_H;
                    break;
                default:
                    break;
            }
            break;

        // Set vertical alignment
        case STYLE_ALIGN_V_PROPERTY:
            switch(variable.type) {
                case STYLESHEET_VARIABLE_DTYPE_TOP:
                    item->verticalAlignment = 0;
                    item->propertyFlags |= PROPERTY_FLAG_ALIGN_V;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_CENTER:
                    item->verticalAlignment = 1;
                    item->propertyFlags |= PROPERTY_FLAG_ALIGN_V;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_BOTTOM:
                    item->verticalAlignment = 2;
                    item->propertyFlags |= PROPERTY_FLAG_ALIGN_V;
                    break;
                default:
                    break;
            }
            break;

        // Set horizontal text alignment
        case STYLE_TEXT_ALIGN_H_PROPERTY:
            switch(variable.type) {
                case STYLESHEET_VARIABLE_DTYPE_LEFT:
                    item->horizontalTextAlignment = 0;
                    item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_H;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_CENTER:
                    item->horizontalTextAlignment = 1;
                    item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_H;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_RIGHT:
                    item->horizontalTextAlignment = 2;
                    item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_H;
                    break;
                default:
                    break;
            }
            break;

        // Set vertical text alignment
        case STYLE_TEXT_ALIGN_V_PROPERTY:
            switch(variable.type) {
                case STYLESHEET_VARIABLE_DTYPE_TOP:
                    item->verticalTextAlignment = 0;
                    item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_V;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_CENTER:
                    item->verticalTextAlignment = 1;
                    item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_V;
                    break;
                case STYLESHEET_VARIABLE_DTYPE_BOTTOM:
                    item->verticalTextAlignment = 2;
                    item->propertyFlags |= PROPERTY_FLAG_TEXT_ALIGN_V;
                    break;
                default:
                    break;
            }
            break;

        // Set absolute position properties
        case STYLE_LEFT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->left = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_LEFT;
            }
            break;

        case STYLE_RIGHT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->right = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_RIGHT;
            }
            break;

        case STYLE_TOP_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->top = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_TOP;
            }
            break;

        case STYLE_BOTTOM_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->bottom = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BOTTOM;
            }
            break;

        // Set background colour
        case STYLE_BACKGROUND_COLOUR_PROPERTY: {
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_RGB) {
                struct RGB rgb = UnpackRGB(variable.value);
                item->backgroundR = rgb.r;
                item->backgroundG = rgb.g;
                item->backgroundB = rgb.b;
                item->propertyFlags |= PROPERTY_FLAG_BACKGROUND;
            }
            else if (variable.type == STYLESHEET_VARIABLE_DTYPE_NONE) {
                item->propertyFlags |= PROPERTY_FLAG_HIDE_BACKGROUND;
                item->layoutFlags |= HIDE_BACKGROUND;
            }
            break;
        }

        // Set border colour
        case STYLE_BORDER_COLOUR_PROPERTY: {
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_RGB) {
                struct RGB rgb = UnpackRGB(variable.value);
                item->borderR = rgb.r;
                item->borderG = rgb.g;
                item->borderB = rgb.b;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_COLOUR;
            }
            break;
        }

        // Set text colour
        case STYLE_TEXT_COLOUR_PROPERTY: {
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_RGB) {
                struct RGB rgb = UnpackRGB(variable.value);
                item->textR = rgb.r;
                item->textG = rgb.g;
                item->textB = rgb.b;
                item->propertyFlags |= PROPERTY_FLAG_TEXT_COLOUR;
            }
            break;
        }

        // Set border width
        case STYLE_BORDER_WIDTH_PROPERTY: {
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                u8 border_width = variable.value;
                item->borderTop = border_width;
                item->borderBottom = border_width;
                item->borderLeft = border_width;
                item->borderRight = border_width;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_TOP;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_BOTTOM;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_LEFT;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RIGHT;
            }
            break;
        }

        case STYLE_BORDER_TOP_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderTop = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_TOP;
            }
            break;

        case STYLE_BORDER_BOTTOM_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderBottom = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_BOTTOM;
            }
            break;

        case STYLE_BORDER_LEFT_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderLeft = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_LEFT;
            }
            break;

        case STYLE_BORDER_RIGHT_WIDTH_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderRight = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RIGHT;
            }
            break;

        // Set border radii
        case STYLE_BORDER_RADIUS_PROPERTY: {
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                u8 border_radius = variable.value;
                item->borderRadiusTl = border_radius;
                item->borderRadiusTr = border_radius;
                item->borderRadiusBl = border_radius;
                item->borderRadiusBr = border_radius;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TL;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TR;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BL;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BR;
            }
            break;
        }

        case STYLE_BORDER_TOP_LEFT_RADIUS_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderRadiusTl = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TL;
            }
            break;

        case STYLE_BORDER_TOP_RIGHT_RADIUS_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderRadiusTr = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_TR;
            }
            break;

        case STYLE_BORDER_BOTTOM_LEFT_RADIUS_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderRadiusBl = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BL;
            }
            break;

        case STYLE_BORDER_BOTTOM_RIGHT_RADIUS_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->borderRadiusBr = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_BORDER_RADIUS_BR;
            }
            break;

        // Set padding
        case STYLE_PADDING_PROPERTY: {
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                u8 pad = variable.value;
                item->padTop = pad;
                item->padBottom = pad;
                item->padLeft = pad;
                item->padRight = pad;
                item->propertyFlags |= PROPERTY_FLAG_PAD_TOP;
                item->propertyFlags |= PROPERTY_FLAG_PAD_BOTTOM;
                item->propertyFlags |= PROPERTY_FLAG_PAD_LEFT;
                item->propertyFlags |= PROPERTY_FLAG_PAD_RIGHT;
            }
            break;
        }

        case STYLE_PADDING_TOP_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->padTop = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_PAD_TOP;
            }
            break;

        case STYLE_PADDING_BOTTOM_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->padBottom = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_PAD_BOTTOM;
            }
            break;

        case STYLE_PADDING_LEFT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->padLeft = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_PAD_LEFT;
            }
            break;

        case STYLE_PADDING_RIGHT_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_NUMBER) {
                item->padRight = variable.value;
                item->propertyFlags |= PROPERTY_FLAG_PAD_RIGHT;
            }
            break;

        case STYLE_FONT_PROPERTY: {
            break;
        }

        case STYLE_INPUT_TYPE_PROPERTY:
            if (variable.type == STYLESHEET_VARIABLE_DTYPE_INPUT_NUMBER) {
                item->propertyFlags |= PROPERTY_FLAG_INPUT_TYPE;
                item->inputType = 1;
            }
            else {
                item->inputType = 0;
            }
            break;

        default:
            break;
    }
}

StyleTextRef* BinarySearchTextRef(Array* textRefs, int targetTokenIndex)
{
    int left = 0;
    int right = textRefs->size - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        StyleTextRef* textRef = (StyleTextRef*)Array_Get(textRefs, mid);
        if (textRef->NU_Token_index == targetTokenIndex) {
            return textRef;  // found
        } else if (textRef->NU_Token_index < targetTokenIndex) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return NULL;  // not found (does not happen)
}

typedef struct FontLoadJob
{
    NU_Font* font;
    int fontSize;
    int fontWeight;
    String filepath;
} FontLoadJob;

typedef struct FontLoaderJobBatch
{
    FontLoadJob* jobs;
    int start;
    int end;
} FontLoaderJobBatch;

int FontLoaderThread(void* data)
{
    FontLoaderJobBatch* batch = (FontLoaderJobBatch*)data;

    bool subpixelRendering = true;
    #ifdef PLATFORM_MACOS
        subpixelRendering = false;
    #endif

    for (int i=batch->start; i<batch->end; i++) {
        FontLoadJob* job = &batch->jobs[i];
        NU_Font_Create(job->font, StringCstr(job->filepath), job->fontSize, job->fontWeight, subpixelRendering);
    }

    return 0;
}

void Stylesheet_Parse_Variable(const char* text, int textLen, int* valueOut, enum StylesheetVariableDtype* typeOut)
{
    struct RGB rgb;
    int _int;
    if (strcmp(text, "v") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_VERTICAL;
    }
    else if (strcmp(text, "h") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_HORIZONTAL;
    }
    else if (strcmp(text, "b") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_BOTH;
    }
    else if (strcmp(text, "s") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_SCROLL;
    }
    else if (strcmp(text, "none") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_NONE;
    }
    else if (strcmp(text, "left") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_LEFT;
    }
    else if (strcmp(text, "right") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_RIGHT;
    }
    else if (strcmp(text, "top") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_TOP;
    }
    else if (strcmp(text, "bottom") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_BOTTOM;
    }
    else if (strcmp(text, "center") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_CENTER;
    }
    else if (strcmp(text, "true") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_TRUE;
    }
    else if (strcmp(text, "false") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_FALSE;
    }
    else if (strcmp(text, "absolute") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_ABSOLUTE;
    }
    else if (strcmp(text, "relative") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_RELATIVE;
    }
    else if (strcmp(text, "number") == 0) {
        *typeOut = STYLESHEET_VARIABLE_DTYPE_INPUT_NUMBER;
    }
    else if (Parse_Hexcode(text, textLen, &rgb)) {
        *valueOut = PackRGB(rgb);
        *typeOut = STYLESHEET_VARIABLE_DTYPE_RGB;
    }
    else if (String_To_Int(&_int, text)) {
        *valueOut = _int;
        *typeOut = STYLESHEET_VARIABLE_DTYPE_NUMBER;
    }
}

static int Stylesheet_Parse_Variables(char* src, TokenArray* tokens, Stylesheet* ss, Array* textRefs, LinearStringmap* variableMap)
{
    int inVariableSelector = 0;
    int i = 0;

    while(i < tokens->size)
    {
        const enum NU_Style_Token token = TokenArray_Get(tokens, i);

        if (token == STYLE_VAR_SELECTOR)
        {
            if (!AssertVariableSelectorGrammar(tokens, i)) return 0;
            inVariableSelector = 1; // Enter @var selector
        }
        else if (token == STYLE_VARIABLE_NAME && inVariableSelector)
        {
            if (!AssertVariableAssignmentGrammar(tokens, i)) return 0;

            // Use binary search to find the desired text refs
            StyleTextRef* varNameTextRef = BinarySearchTextRef(textRefs, i);
            StyleTextRef* valTextRef = BinarySearchTextRef(textRefs, i+2);

            // If text ref -> parse variable
            if (varNameTextRef && valTextRef)
            {
                // Get variable name and value strings
                char* varName = &src[varNameTextRef->src_index];
                char* valText = &src[valTextRef->src_index];

                // Try to parse variable init value (store bytes as an int and infer type)
                StylesheetVariable newVar;
                newVar.type = STYLESHEET_VARIABLE_DTYPE_UNKNOWN;
                newVar.value = 0;
                Stylesheet_Parse_Variable(valText, valTextRef->char_count, &newVar.value, &newVar.type);
                newVar.value_DEFAULT = newVar.value;
                newVar.type_DEFAULT = newVar.type;

                // Variable exists -> overwrite value
                u16* existingVarIndex = LinearStringmap_Get(variableMap, varName);
                if (existingVarIndex) {
                    StylesheetVariable* variable = Array_Get(&ss->variables, *existingVarIndex);
                    *variable = newVar;
                }
                // Variable does not exist -> create it
                else {
                    // Add variable
                    Array_Push(&ss->variables, &newVar);
                    u16 index = ss->variables.size - 1;

                    // Add index to variableMap
                    LinearStringmap_Set(variableMap, varName, &index);
                }
            }
            i += 3; continue; // ^
        }
        else if (token == STYLE_SELECTOR_CLOSE_BRACE) inVariableSelector = 0; // Exit @var selector
        i += 1; // ^
    }

    return 1;
}

static int Stylesheet_Parse_Screen_Queries(char* src, TokenArray* tokens, Stylesheet* ss, Array* textRefs, LinearStringmap* variableMap)
{
    StylesheetScreenQuery* screenQuery = NULL;
    int inScreenQuerySelector = 0;
    int i = 0;

    while(i < tokens->size)
    {
        const enum NU_Style_Token token = TokenArray_Get(tokens, i);

        if (token == STYLE_SCREEN_SELECTOR) {

            if (!AssertScreenSelectorGrammar(tokens, i)) return 0;

            enum NU_Style_Token comparatorToken = TokenArray_Get(tokens, i+1);
            enum NU_Style_Token screenWidthToken = TokenArray_Get(tokens, i+2);

            // Get screen width text ref
            StyleTextRef* textRef = BinarySearchTextRef(textRefs, i+2);
            const char* text = &src[textRef->src_index];
            int screenWidth;
            if (String_To_Int(&screenWidth, text)) {

                // Add screen query
                screenQuery = Array_PushEmpty(&ss->screenQueries);
                screenQuery->screenWidth = screenWidth;
                screenQuery->comparator = comparatorToken;
                screenQuery->overrideArrayPartitionStart = ss->variableOverrides.size;
                screenQuery->overrideArrayPartitionCount = 0;
            }
            else {
                char errMessage[128];
                snprintf(errMessage, sizeof(errMessage), "<CSS Error> Expected screen width (int), got \"%s\" ','", text);
                return 0;
            }

            inScreenQuerySelector = 1;
            i += 4; continue;
        }
        else if (token == STYLE_VARIABLE_NAME && inScreenQuerySelector) {

            if (!AssertVariableAssignmentGrammar(tokens, i)) return 0;

            // Use binary search to find the desired text refs
            StyleTextRef* varNameTextRef = BinarySearchTextRef(textRefs, i);
            StyleTextRef* valTextRef = BinarySearchTextRef(textRefs, i+2);

            // If text ref -> parse variable
            if (varNameTextRef && valTextRef) {

                // Get variable name
                char* varName = &src[varNameTextRef->src_index];
                char* valText = &src[valTextRef->src_index];

                // Get existing variable index
                u16* existingVarIndex = LinearStringmap_Get(variableMap, varName);
                if (existingVarIndex) {
                    StylesheetVariable* variable = Array_Get(&ss->variables, *existingVarIndex);

                    // Try parse variable, store as packed int and infer type
                    StylesheetVariableOverride varOverride;
                    varOverride.type_OVERRIDE = STYLESHEET_VARIABLE_DTYPE_UNKNOWN;
                    varOverride.value_OVERRIDE = 0;
                    varOverride.variableIndex = *existingVarIndex;
                    Stylesheet_Parse_Variable(valText, valTextRef->char_count, &varOverride.value_OVERRIDE, &varOverride.type_OVERRIDE);

                    // Add variable override
                    Array_Push(&ss->variableOverrides, &varOverride);
                    screenQuery->overrideArrayPartitionCount++;
                }
                // Variable not initialised (not found in @var) -> error
                else {
                    char errMessage[128];
                    snprintf(errMessage, sizeof(errMessage), "<CSS Error> Variable \"%s\" uninitialised! Add this variable to @var','", varName);
                    return 0;
                }
            }
            i += 3; continue; // ^
        }
        else if (token == STYLE_SELECTOR_CLOSE_BRACE) inScreenQuerySelector = 0; // Exit @screen selector
        i += 1;
    }

    return 1;
}

static int Stylesheet_Parse_Fonts(Stylesheet* ss, char* src, TokenArray* tokens, Array* textRefs)
{
    int inFontSelector = 0;
    int fontSize = 18;
    int fontWeight = 400;
    char* fontName = NULL;
    char* fontSrc = NULL;
    FontLoadJob fontJobs[128]; int fontJobCount = 0;
    int i = 0;

    while(i < tokens->size)
    {
        const enum NU_Style_Token token = TokenArray_Get(tokens, i);

        if (token == STYLE_FONT_CREATION_SELECTOR)
        {
            if (!AssertFontCreationSelectorGrammar(tokens, i)) return 0;

            StyleTextRef* textRef = BinarySearchTextRef(textRefs, i+1);
            if (textRef) fontName = &src[textRef->src_index];
            inFontSelector = 1; // Enter @font selector
            i += 3; continue; // ^
        }
        else if (NU_Is_Property_Identifier_Token(token) && inFontSelector)
        {
            if (!AssertPropertyIdentifierGrammar(tokens, i)) return 0;

            // Use binary search to find the corresponding property text
            StyleTextRef* textRef = BinarySearchTextRef(textRefs, i + 2);

            if (textRef)
            {
                char* propertyText = &src[textRef->src_index];

                switch (token)
                {
                    case STYLE_FONT_SRC: {
                        fontSrc = propertyText;
                        break;
                    }
                    case STYLE_FONT_SIZE: {
                        int size = 0;
                        if (String_To_Int(&size, propertyText)) fontSize = size;
                        break;
                    }
                    case STYLE_FONT_WEIGHT: {
                        int weight = 0;
                        if (String_To_Int(&weight, propertyText)) fontWeight = weight;
                        break;
                    }
                    default:
                        break;
                }
            }
            i += 3; continue;
        }
        else if (token == STYLE_SELECTOR_CLOSE_BRACE && inFontSelector) {

            if (fontSrc != NULL) {

                // Create a new font
                void* found_font = LinearStringmap_Get(&ss->fontNameIndexMap, fontName);
                if (found_font == NULL && fontJobCount < 128)
                {
                    // Create uninitialised font
                    NU_Font font;
                    u8 createFontID = Container_Add(&ss->fonts, &font);
                    NU_Font* createFont = Container_Get(&ss->fonts, createFontID);
                    LinearStringmap_Set(&ss->fontNameIndexMap, fontName, &createFontID);

                    // Create a font job
                    fontJobs[fontJobCount].filepath = StringCreate(fontSrc);
                    fontJobs[fontJobCount].fontSize = fontSize;
                    fontJobs[fontJobCount].fontWeight = fontWeight;
                    fontJobs[fontJobCount].font = createFont;
                    fontJobCount++;
                }
            }
            else {
                // Free filepath strings
                for (int i=0; i<fontJobCount; i++) {
                    StringFree(fontJobs[i].filepath);
                }
                return 0;
            }

            inFontSelector = 0; // Exit @font selector
        }
        i += 1;
    }

    // Create fonts in parallel
    int threadCount = SDL_GetNumLogicalCPUCores();
    if (threadCount <= 0) threadCount = 1;
    if (threadCount > fontJobCount) threadCount = fontJobCount;
    SDL_Thread* threads[128];
    FontLoaderJobBatch batches[128];

    // Create job batches
    int jobsPerThread = fontJobCount / threadCount;
    int remainder = fontJobCount % threadCount;
    int start = 0;
    for (int t=0; t<threadCount; t++) {
        int count = jobsPerThread + (t < remainder ? 1 : 0);
        batches[t].jobs = fontJobs;
        batches[t].start = start;
        batches[t].end = start + count;
        start += count;
    }

    // Dispatch work
    for (int t=0; t<threadCount; t++) {
        threads[t] = SDL_CreateThread(FontLoaderThread, "FontLoader", &batches[t]);
    }

    // Wait for work to finish
    for (int t = 0; t < threadCount; t++) {
        SDL_WaitThread(threads[t], NULL);
    }

    // Free filepath strings
    for (int i=0; i<fontJobCount; i++) {
        StringFree(fontJobs[i].filepath);
    }

    // Upload all font atlases to the GPU
    for (int i=0; i<ss->fonts.size; i++) {
        NU_Font* font = Container_GetAt(&ss->fonts, i);
        NU_Font_Atlas_Upload_Or_Modify_GPU(&font->atlas);
    }

    return 1;
}

static int Stylesheet_Parse_Default(char* src, TokenArray* tokens, Array* textRefs, LinearStringmap* variableMap, Stylesheet* ss, ImageResourceLoader* imageResourceLoader)
{
    int inDefaultSelector = 0;
    int i = 0;

    while(i < tokens->size)
    {
        const enum NU_Style_Token token = TokenArray_Get(tokens, i);

        if (token == STYLE_DEFAULT_SELECTOR)
        {
            if (!AssertDefaultSelectorGrammar(tokens, i)) return 0; // Enter @default selector
            inDefaultSelector = 1;
        }
        else if (NU_Is_Property_Identifier_Token(token) && inDefaultSelector)
        {
            if (!AssertPropertyIdentifierGrammar(tokens, i)) return 0;

            // Use binary search to find the corresponding property text
            StyleTextRef* textRef = BinarySearchTextRef(textRefs, i+2);

            // Variable property
            enum NU_Style_Token secondNextToken = TokenArray_Get(tokens, i+2);
            if (secondNextToken == STYLE_VARIABLE_PROPERTY_VALUE) {

                char* variableName = &src[textRef->src_index];
                u16* variableIndex = LinearStringmap_Get(variableMap, variableName);

                // Error! variable not found
                if (!variableIndex) {
                    char errMessage[128];
                    snprintf(errMessage, sizeof(errMessage), "<CSS Error> Variable \"%s\" is undefined", variableName);
                    ErrorSystem_AddError(&GUI.errorSystem, errMessage);
                    return 0;
                }

                StylesheetVariable variable = *(StylesheetVariable*) Array_Get(&ss->variables, *variableIndex);
                Stylesheet_Parse_Variable_Property(ss, token, &ss->defaultStyleItem, variable, imageResourceLoader);
            }
            // If text ref -> parse property
            else if (textRef) {
                char* text = &src[textRef->src_index];
                Stylesheet_Parse_Property(ss, token, &ss->defaultStyleItem, text, textRef->char_count, imageResourceLoader);
            }

            i += 3; continue;
        }
        else if (token == STYLE_SELECTOR_CLOSE_BRACE) inDefaultSelector = 0; // Exit @default selector
        i += 1;
    }

    return 1;
}

void Stylesheet_Parse_Scrollbar_Property(Stylesheet* ss, const enum NU_Style_Token token, const char* text)
{
    u8 property;
    switch (token)
    {
        case STYLE_WIDTH_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.width = property;
            }
            break;
        case STYLE_HEIGHT_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.height = property;
            }
            break;
        case STYLE_SCROLLBAR_OVERLAY:
            if (stringEquals(text, "true")) {
                ss->scrollbarStyle.overlay = true;
            }
            break;
        default:
            break;
    }
}

void Stylesheet_Parse_Scroll_Thumb_Property(Stylesheet* ss, const enum NU_Style_Token token, const char* text, int textLen)
{
    u8 property;
    struct RGB rgb;
    switch (token)
    {
        case STYLE_BACKGROUND_COLOUR_PROPERTY:
            if (Parse_Hexcode(text, textLen, &rgb)) {
                ss->scrollbarStyle.thumbBackgroundR = rgb.r;
                ss->scrollbarStyle.thumbBackgroundG = rgb.g;
                ss->scrollbarStyle.thumbBackgroundB = rgb.b;
            }
            break;
        case STYLE_BORDER_COLOUR_PROPERTY:
            if (Parse_Hexcode(text, textLen, &rgb)) {
                ss->scrollbarStyle.thumbBorderR = rgb.r;
                ss->scrollbarStyle.thumbBorderG = rgb.g;
                ss->scrollbarStyle.thumbBorderB = rgb.b;
            }
            break;
        case STYLE_BORDER_WIDTH_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.thumbBorderTop    = property;
                ss->scrollbarStyle.thumbBorderBottom = property;
                ss->scrollbarStyle.thumbBorderLeft   = property;
                ss->scrollbarStyle.thumbBorderRight  = property;
            }
            break;
        case STYLE_BORDER_TOP_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderTop, text); // 0 if parse fails
            break;
        case STYLE_BORDER_BOTTOM_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderBottom, text); // 0 if parse fails
            break;
        case STYLE_BORDER_LEFT_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderLeft, text); // 0 if parse fails
            break;
        case STYLE_BORDER_RIGHT_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderRight, text); // 0 if parse fails
            break;
        case STYLE_BORDER_RADIUS_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.thumbBorderRadiusTl = property;
                ss->scrollbarStyle.thumbBorderRadiusTr = property;
                ss->scrollbarStyle.thumbBorderRadiusBl = property;
                ss->scrollbarStyle.thumbBorderRadiusBr = property;
            }
            break;
        case STYLE_BORDER_TOP_LEFT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderRadiusTl, text); // 0 if parse fails
            break;
        case STYLE_BORDER_TOP_RIGHT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderRadiusTr, text); // 0 if parse fails
            break;
        case STYLE_BORDER_BOTTOM_LEFT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderRadiusBl, text); // 0 if parse fails
            break;
        case STYLE_BORDER_BOTTOM_RIGHT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.thumbBorderRadiusBr, text); // 0 if parse fails
            break;
        case STYLE_SCROLL_THUMB_MIN_SIZE:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.thumbMinSize = property;
            }
            break;
        default: // Invalid property (ignore)
            break;
    }
}

void Stylesheet_Parse_Scroll_Track_Property(Stylesheet* ss, const enum NU_Style_Token token, const char* text, int textLen)
{
    u8 property;
    struct RGB rgb;
    switch (token)
    {
        case STYLE_BACKGROUND_COLOUR_PROPERTY:
            if (Parse_Hexcode(text, textLen, &rgb)) {
                ss->scrollbarStyle.trackBackgroundR = rgb.r;
                ss->scrollbarStyle.trackBackgroundG = rgb.g;
                ss->scrollbarStyle.trackBackgroundB = rgb.b;
            }
            break;
        case STYLE_BORDER_COLOUR_PROPERTY:
            if (Parse_Hexcode(text, textLen, &rgb)) {
                ss->scrollbarStyle.trackBorderR = rgb.r;
                ss->scrollbarStyle.trackBorderG = rgb.g;
                ss->scrollbarStyle.trackBorderB = rgb.b;
            }
            break;
        case STYLE_BORDER_WIDTH_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.trackBorderTop    = property;
                ss->scrollbarStyle.trackBorderBottom = property;
                ss->scrollbarStyle.trackBorderLeft   = property;
                ss->scrollbarStyle.trackBorderRight  = property;
            }
            break;
        case STYLE_BORDER_TOP_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderTop, text);
            break;
        case STYLE_BORDER_BOTTOM_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderBottom, text);
            break;
        case STYLE_BORDER_LEFT_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderLeft, text);
            break;
        case STYLE_BORDER_RIGHT_WIDTH_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderRight, text);
            break;
        case STYLE_BORDER_RADIUS_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.trackBorderRadiusTl = property;
                ss->scrollbarStyle.trackBorderRadiusTr = property;
                ss->scrollbarStyle.trackBorderRadiusBl = property;
                ss->scrollbarStyle.trackBorderRadiusBr = property;
            }
            break;
        case STYLE_BORDER_TOP_LEFT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderRadiusTl, text);
            break;
        case STYLE_BORDER_TOP_RIGHT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderRadiusTr, text);
            break;
        case STYLE_BORDER_BOTTOM_LEFT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderRadiusBl, text);
            break;
        case STYLE_BORDER_BOTTOM_RIGHT_RADIUS_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackBorderRadiusBr, text);
            break;
        case STYLE_PADDING_PROPERTY:
            if (String_To_u8(&property, text)) {
                ss->scrollbarStyle.trackPadTop    = property;
                ss->scrollbarStyle.trackPadBottom = property;
                ss->scrollbarStyle.trackPadLeft   = property;
                ss->scrollbarStyle.trackPadRight  = property;
            }
            break;
        case STYLE_PADDING_TOP_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackPadTop, text);
            break;
        case STYLE_PADDING_BOTTOM_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackPadBottom, text);
            break;
        case STYLE_PADDING_LEFT_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackPadLeft, text);
            break;
        case STYLE_PADDING_RIGHT_PROPERTY:
            String_To_u8(&ss->scrollbarStyle.trackPadRight, text);
            break;
        default: // Invalid property (ignore)
            break;
    }
}

enum StylesheetParseCtx
{
    STYLE_PARSE_CTX_SELECTOR,
    STYLE_PARSE_CTX_VAR_SELECTOR,
    STYLE_PARSE_CTX_SCREEN_SELECTOR,
    STYLE_PARSE_CTX_FONT_SELECTOR,
    STYLE_PARSE_CTX_DEFAULT_SELECTOR,
    STYLE_PARSE_CTX_SCROLLBAR_SELECTOR,
    STYLE_PARSE_CTX_SCROLL_THUMB_SELECTOR,
    STYLE_PARSE_CTX_SCROLL_TRACK_SELECTOR,
};

static int Stylesheet_Parse(char* src, TokenArray* tokens, Array* textRefs, LinearStringmap* variableMap, Stylesheet* ss, ImageResourceLoader* imageResourceLoader)
{
    // Parse Variables
    if (!Stylesheet_Parse_Variables(src, tokens, ss, textRefs, variableMap)) return 0;

    // Parse Screen Queries
    if (!Stylesheet_Parse_Screen_Queries(src, tokens, ss, textRefs, variableMap)) return 0;

    // Parse Fonts
    if (!Stylesheet_Parse_Fonts(ss, src, tokens, textRefs)) return 0;

    // Parse Default
    if (!Stylesheet_Parse_Default(src, tokens, textRefs, variableMap, ss, imageResourceLoader)) return 0;

    // ----------------------
    // --- Parser Context ---
    // ----------------------
    /* 0 == standard selector; 1 == font creation selector; 2 == default selector;
       3 == scrollbar selector; 4 == scroll thumb selector; 5 == scroll track selector;
       6 == var selector; 7 == screen selector;
    */
    enum StylesheetParseCtx ctx = STYLE_PARSE_CTX_SELECTOR;
    u32 selectorIndexes[64];
    int selectorCount = 0;
    int succeeded = 1;

    // ------------------------------
    // --- Text Reference Context ---
    // ------------------------------
    u32 textRefIndex = 0;
    StyleTextRef* textRef;

    // --------------------------
    // --- Working Style Item ---
    // --------------------------
    Stylesheet_Item item;
    item.propertyFlags = 0;
    item.fontId = 0;

    // -------------
    // --- Parse ---
    // -------------
    int i = 0;
    while(i < tokens->size)
    {
        const enum NU_Style_Token token = TokenArray_Get(tokens, i);

        if (token == STYLE_FONT_CREATION_SELECTOR) {
            // Grammar already checked
            textRefIndex += 1;
            ctx = STYLE_PARSE_CTX_FONT_SELECTOR;
            i += 2;
            continue;
        }
        else if (token == STYLE_DEFAULT_SELECTOR) {
            // Grammar already checked
            ctx = STYLE_PARSE_CTX_DEFAULT_SELECTOR;
            i += 2;
            continue;
        }
        else if (token == STYLE_SCROLLBAR_SELECTOR) {
            if (!AssertScrollSelectorGrammar(tokens, i)) return 0;
            ctx = STYLE_PARSE_CTX_SCROLLBAR_SELECTOR;
            i += 2;
            continue;
        }
        else if (token == STYLE_SCROLL_THUMB_SELECTOR) {
            if (!AssertScrollSelectorGrammar(tokens, i)) return 0;
            ctx = STYLE_PARSE_CTX_SCROLL_THUMB_SELECTOR;
            i += 2;
            continue;
        }
        else if (token == STYLE_SCROLL_TRACK_SELECTOR) {
            if (!AssertScrollSelectorGrammar(tokens, i)) return 0;
            ctx = STYLE_PARSE_CTX_SCROLL_TRACK_SELECTOR;
            i += 2;
            continue;
        }
        else if (token == STYLE_VAR_SELECTOR) {
            // Grammar already checked
            ctx = STYLE_PARSE_CTX_VAR_SELECTOR;
            i += 2;
            continue;
        }
        else if (token == STYLE_SCREEN_SELECTOR) {
            // Grammar already checked
            textRefIndex += 1; // skip screen width value text e.g. "1080"
            ctx = STYLE_PARSE_CTX_SCREEN_SELECTOR;
            i += 4;
            continue;
        }
        else if (token == STYLE_VARIABLE_NAME) {
            // Grammar already checked
            textRefIndex += 2; // skip name and value text e.g. "--var" "200"
            i += 3;
            continue;
        }

        else if (token == STYLE_SELECTOR_OPEN_BRACE) {
            if (AssertSelectionOpeningBraceGrammar(tokens, i)) {
                item.propertyFlags = 0;
                item.layoutFlags = 0;
                item.fontId = ss->defaultStyleItem.fontId;
                i += 1;
                continue;
            }
            else {
                succeeded = 0; break;
            }
        }
        else if (token == STYLE_SELECTOR_CLOSE_BRACE) {
            if (!AssertSelectionClosingBraceGrammar(tokens, i)) { succeeded = 0; break; }

            if (ctx == 0) {
                for (int j=0; j<selectorCount; j++) {
                    u32 item_index = selectorIndexes[j];
                    Stylesheet_Item* curr_item = Array_Get(&ss->items, item_index);

                    // Update current item
                    Stylesheet_Overwrite_Style_Item(curr_item, &item);
                }
                selectorCount = 0;
            }

            ctx = 0;
            i += 1;
            continue;
        }
        else if (NU_Is_Tag_Selector_Token(token)) {
            if (i < tokens->size - 1) {
                enum NU_Style_Token next_token = TokenArray_Get(tokens, i+1);
                if (next_token == STYLE_SELECTOR_COMMA || next_token == STYLE_SELECTOR_OPEN_BRACE)
                {
                    int tag = NU_Token_To_Tag(token);
                    void* found = Hashmap_Get(&ss->tag_item_hashmap, &tag);

                    // Style item exists
                    if (found != NULL) {
                        Stylesheet_Item* found_item = Array_Get(&ss->items, *(u32*)found);
                        selectorIndexes[selectorCount] = *(u32*)found;
                    }
                    // Style item does not exist -> add one
                    else {
                        Stylesheet_Item new_item;
                        new_item.class = NULL;
                        new_item.id = NULL;
                        new_item.tag = tag;
                        new_item.propertyFlags = 0;
                        Array_Push(&ss->items, &new_item);
                        selectorIndexes[selectorCount] = (u32)(ss->items.size - 1);
                        Hashmap_Set(&ss->tag_item_hashmap, &tag, &selectorIndexes[selectorCount]); // Store item index
                    }

                    i += 1;
                    selectorCount++;
                    continue;
                }
                else if (next_token == STYLE_PSEUDO_COLON && i < tokens->size - 3)
                {
                    enum NU_Style_Token following_token = TokenArray_Get(tokens, i+2);
                    enum NU_Style_Token third_token = TokenArray_Get(tokens, i+3);
                    if (NU_Is_Pseudo_Token(following_token) && (third_token == STYLE_SELECTOR_COMMA || third_token == STYLE_SELECTOR_OPEN_BRACE))
                    {
                        int tag = NU_Token_To_Tag(token);
                        enum NU_Pseudo_Class pseudo_class = Token_To_Pseudo_Class(following_token);

                        // Construct key
                        struct Stylesheet_Tag_Pseudo_Pair key = { tag, pseudo_class };

                        // Query hashmap
                        void* found = Hashmap_Get(&ss->tag_pseudo_item_hashmap, &key);

                        // Style item exists
                        if (found != NULL)
                        {
                            Stylesheet_Item* found_item = Array_Get(&ss->items, *(u32*)found);
                            selectorIndexes[selectorCount] = *(u32*)found;
                        }
                        // Style item does not exist -> add one
                        else
                        {
                            Stylesheet_Item new_item;
                            new_item.class = NULL;
                            new_item.id = NULL;
                            new_item.tag = tag;
                            new_item.propertyFlags = 0;
                            Array_Push(&ss->items, &new_item);
                            selectorIndexes[selectorCount] = (u32)(ss->items.size - 1);
                            Hashmap_Set(&ss->tag_pseudo_item_hashmap, &key, &selectorIndexes[selectorCount]); // Store item index
                        }
                    }
                    else {
                        succeeded = 0; break;
                    }

                    i += 3;
                    selectorCount++;
                    continue;
                }
                else {
                    succeeded = 0; break;
                }
            }
            else {
                succeeded = 0; break;
            }
        }

        else if (token == STYLE_CLASS_SELECTOR)
        {
            if (i < tokens->size - 1)
            {
                enum NU_Style_Token next_token = TokenArray_Get(tokens, i+1);
                if (next_token == STYLE_SELECTOR_COMMA || next_token == STYLE_SELECTOR_OPEN_BRACE)
                {
                    // Get class string
                    textRef = (StyleTextRef*)Array_Get(textRefs, textRefIndex++);
                    char* src_class = &src[textRef->src_index];

                    // Get stored class
                    char* stored_class = LinearStringset_Get(&ss->class_string_set, src_class);

                    // If style item exists
                    void* found = Hashmap_Get(&ss->class_item_hashmap, &stored_class);
                    if (found != NULL)
                    {
                        Stylesheet_Item* found_item = Array_Get(&ss->items, *(u32*)found);
                        selectorIndexes[selectorCount] = *(u32*)found;
                    }
                    else // does not exist -> add item
                    {
                        // Add class to string set
                        LinearStringset_Add(&ss->class_string_set, src_class);
                        stored_class = LinearStringset_Get(&ss->class_string_set, src_class);

                        // Add style item for class
                        Stylesheet_Item new_item;
                        new_item.class = stored_class;
                        new_item.id = NULL;
                        new_item.tag = -1;
                        new_item.propertyFlags = 0;
                        Array_Push(&ss->items, &new_item);
                        selectorIndexes[selectorCount] = (u32)(ss->items.size - 1);
                        Hashmap_Set(&ss->class_item_hashmap, &stored_class, &selectorIndexes[selectorCount]); // Store item index
                    }

                    i += 1;
                    selectorCount++;
                    continue;
                }
                else if (next_token == STYLE_PSEUDO_COLON && i < tokens->size-3)
                {
                    enum NU_Style_Token following_token = TokenArray_Get(tokens, i+2);
                    enum NU_Style_Token third_token = TokenArray_Get(tokens, i+3);
                    if (NU_Is_Pseudo_Token(following_token) && (third_token == STYLE_SELECTOR_COMMA || third_token == STYLE_SELECTOR_OPEN_BRACE))
                    {
                        enum NU_Pseudo_Class pseudo_class = Token_To_Pseudo_Class(following_token);

                        // Get class string
                        textRef = (StyleTextRef*)Array_Get(textRefs, textRefIndex++);
                        char* src_class = &src[textRef->src_index];

                        // Get stored class
                        char* stored_class = LinearStringset_Get(&ss->class_string_set, src_class);

                        if (stored_class != NULL)
                        {
                            // Get stored class pseudo
                            struct Stylesheet_String_Pseudo_Pair key = { stored_class, pseudo_class };
                            void* found = Hashmap_Get(&ss->class_pseudo_item_hashmap, &key);

                            // No pseudo item exists for this class
                            if (found == NULL)
                            {
                                // Add pseudo style item
                                Stylesheet_Item new_item;
                                new_item.class = stored_class;
                                new_item.id = NULL;
                                new_item.tag = -1;
                                new_item.propertyFlags = 0;
                                Array_Push(&ss->items, &new_item);
                                selectorIndexes[selectorCount] = (u32)(ss->items.size - 1);
                                Hashmap_Set(&ss->class_pseudo_item_hashmap, &key, &selectorIndexes[selectorCount]); // Store item index
                            }
                            // Item found
                            else
                            {
                                Stylesheet_Item* found_item = Array_Get(&ss->items, *(u32*)found);
                                selectorIndexes[selectorCount] = *(u32*)found;
                            }
                        }
                    }
                    else {
                        succeeded = 0; break;
                    }

                    i += 3;
                    selectorCount++;
                    continue;
                }
                else {
                    succeeded = 0; break;
                }
            }
            else {
                succeeded = 0; break;
            }
        }

        else if (token == STYLE_ID_SELECTOR)
        {
            if (i < tokens->size - 1)
            {
                enum NU_Style_Token next_token = TokenArray_Get(tokens, i+1);
                if (next_token == STYLE_SELECTOR_COMMA || next_token == STYLE_SELECTOR_OPEN_BRACE)
                {
                    // Get id string
                    textRef = (StyleTextRef*)Array_Get(textRefs, textRefIndex++);
                    char* src_id = &src[textRef->src_index];

                    // Get stored id
                    char* stored_id = LinearStringset_Get(&ss->id_string_set, src_id);

                    // If style item exists
                    void* found = Hashmap_Get(&ss->id_item_hashmap, &stored_id);
                    if (found != NULL)
                    {
                        Stylesheet_Item* found_item = Array_Get(&ss->items, *(u32*)found);
                        selectorIndexes[selectorCount] = *(u32*)found;
                    }
                    else // does not exist -> add item
                    {
                        // Add class to string set
                        LinearStringset_Add(&ss->id_string_set, src_id);
                        stored_id = LinearStringset_Get(&ss->id_string_set, src_id);

                        // Add style item for id
                        Stylesheet_Item new_item;
                        new_item.class = NULL;
                        new_item.id = stored_id;
                        new_item.tag = -1;
                        new_item.propertyFlags = 0;
                        Array_Push(&ss->items, &new_item);
                        selectorIndexes[selectorCount] = (u32)(ss->items.size - 1);
                        Hashmap_Set(&ss->id_item_hashmap, &stored_id, &selectorIndexes[selectorCount]); // Store item index
                        LinearStringset_Add(&ss->id_string_set, src_id);
                    }

                    i += 1;
                    selectorCount++;
                    continue;
                }
                else if (next_token == STYLE_PSEUDO_COLON && i < tokens->size-3)
                {
                    enum NU_Style_Token following_token = TokenArray_Get(tokens, i+2);
                    enum NU_Style_Token third_token = TokenArray_Get(tokens, i+3);
                    if (NU_Is_Pseudo_Token(following_token) && (third_token == STYLE_SELECTOR_COMMA || third_token == STYLE_SELECTOR_OPEN_BRACE))
                    {
                        enum NU_Pseudo_Class pseudo_class = Token_To_Pseudo_Class(following_token);

                        // Get id string
                        textRef = (StyleTextRef*)Array_Get(textRefs, textRefIndex++);
                        char* src_id = &src[textRef->src_index];

                        // Get stored id
                        char* stored_id = LinearStringset_Get(&ss->id_string_set, src_id);

                        if (stored_id != NULL)
                        {
                            // Get stored id pseudo
                            struct Stylesheet_String_Pseudo_Pair key = { stored_id, pseudo_class };
                            void* found = Hashmap_Get(&ss->id_pseudo_item_hashmap, &key);

                            // No pseudo item exists for this id
                            if (found == NULL)
                            {
                                // Add pseudo style item
                                Stylesheet_Item new_item;
                                new_item.class = NULL;
                                new_item.id = stored_id;
                                new_item.tag = -1;
                                new_item.propertyFlags = 0;
                                Array_Push(&ss->items, &new_item);
                                selectorIndexes[selectorCount] = (u32)(ss->items.size - 1);
                                Hashmap_Set(&ss->id_pseudo_item_hashmap, &key, &selectorIndexes[selectorCount]); // Store item index
                            }
                            // Item found
                            else
                            {
                                Stylesheet_Item* found_item = Array_Get(&ss->items, *(u32*)found);
                                selectorIndexes[selectorCount] = *(u32*)found;
                            }
                        }
                    }
                    else {
                        succeeded = 0; break;
                    }

                    i += 3;
                    selectorCount++;
                    continue;
                }
                else {
                    succeeded = 0; break;
                }
            }
            else {
                succeeded = 0; break;
            }
        }

        else if (token == STYLE_SELECTOR_COMMA)
        {
            if (AssertSelectorCommaGrammar(tokens, i)) {
                if (selectorCount == 64) {
                    succeeded = 0; break;
                }
                i += 1;
                continue;
            }
            else {
                succeeded = 0; break;
            }
        }

        else if (NU_Is_Property_Identifier_Token(token))
        {
            // Property Identifier Assertion
            if (!AssertPropertyIdentifierGrammar(tokens, i)) return 0;

            // Get property / variable text ref
            textRef = (StyleTextRef*)Array_Get(textRefs, textRefIndex++);
            char* text = &src[textRef->src_index];

            // Variable property
            enum NU_Style_Token secondNextToken = TokenArray_Get(tokens, i+2);
            if (secondNextToken == STYLE_VARIABLE_PROPERTY_VALUE && (ctx == 0 || ctx == 3 || ctx == 4 || ctx == 5)) {

                const char* variableName = text;
                u16* varIndex = LinearStringmap_Get(variableMap, variableName);

                // Error! property value not assigned
                if (!varIndex) {
                    char errMessage[128];
                    snprintf(errMessage, sizeof(errMessage), "<CSS Error> Variable \"%s\" is undefined", variableName);
                    ErrorSystem_AddError(&GUI.errorSystem, errMessage);
                    return 0;
                }

                StylesheetVariable variable = *(StylesheetVariable*) Array_Get(&ss->variables, *varIndex);
                Stylesheet_Parse_Variable_Property(ss, token, &item, variable, imageResourceLoader);
            }
            // Hardcoded property
            else {
                switch (ctx) {
                    case STYLE_PARSE_CTX_SELECTOR:
                        Stylesheet_Parse_Property(ss, token, &item, text, textRef->char_count, imageResourceLoader);
                        break;
                    case STYLE_PARSE_CTX_SCROLLBAR_SELECTOR:
                        Stylesheet_Parse_Scrollbar_Property(ss, token, text);
                        break;
                    case STYLE_PARSE_CTX_SCROLL_THUMB_SELECTOR:
                        Stylesheet_Parse_Scroll_Thumb_Property(ss, token, text, textRef->char_count);
                        break;
                    case STYLE_PARSE_CTX_SCROLL_TRACK_SELECTOR:
                        Stylesheet_Parse_Scroll_Track_Property(ss, token, text, textRef->char_count);
                        break;
                    default:
                        break;
                }
            }

            i += 3;
            continue;
        }

        else {
            i += 1; continue;
        }
    }

    // No fonts loaded -> default load embedded font
    if (ss->fonts.size == 0) {

        bool subpixelRendering = true;
        #ifdef PLATFORM_MACOS
            subpixelRendering = false;
        #endif

        NU_Font font;
        if (NU_Font_Create_Default(&font, 14, 400, subpixelRendering)) {
            int id = Container_Add(&ss->fonts, &font); // FontID will be 0
            NU_Font* font = Container_Get(&ss->fonts, id);
            NU_Font_Atlas_Upload_Or_Modify_GPU(&font->atlas);
        }
        else succeeded = 0;
    }

    return succeeded;
}
