#pragma once

#include "nu_stylesheet_tokens.h"
typedef struct Stylesheet_Tag_Pseudo_Pair
{
    int tag;
    int pseudo_class;
} Stylesheet_Tag_Pseudo_Pair;

typedef struct Stylesheet_String_Pseudo_Pair
{
    char* string;
    int pseudo_class;
} Stylesheet_String_Pseudo_Pair;

typedef struct Stylesheet_Item
{
    uint64_t propertyFlags;
    char* class;
    char* id;
    int tag;
    int item_index;
    int imageHandle;
    u16 prefWidth, prefHeight;
    u16 minWidth, maxWidth, minHeight, maxHeight;
    i16 left, right, top, bottom;
    u16 layoutFlags;
    u8 gap, padTop, padBottom, padLeft, padRight;
    u8 borderTop, borderBottom, borderLeft, borderRight;
    u8 borderRadiusTl, borderRadiusTr, borderRadiusBl, borderRadiusBr;
    u8 backgroundR, backgroundG, backgroundB;
    u8 borderR, borderG, borderB;
    u8 textR, textG, textB;
    u8 fontId;
    char horizontalAlignment;
    char verticalAlignment;
    char horizontalTextAlignment;
    char verticalTextAlignment;
    u8 inputType;
} Stylesheet_Item;

typedef enum StylesheetVariableDtype
{
    STYLESHEET_VARIABLE_DTYPE_RGB,
    STYLESHEET_VARIABLE_DTYPE_NUMBER,
    STYLESHEET_VARIABLE_DTYPE_TOP,
    STYLESHEET_VARIABLE_DTYPE_BOTTOM,
    STYLESHEET_VARIABLE_DTYPE_LEFT,
    STYLESHEET_VARIABLE_DTYPE_RIGHT,
    STYLESHEET_VARIABLE_DTYPE_CENTER,
    STYLESHEET_VARIABLE_DTYPE_VERTICAL,
    STYLESHEET_VARIABLE_DTYPE_HORIZONTAL,
    STYLESHEET_VARIABLE_DTYPE_BOTH,
    STYLESHEET_VARIABLE_DTYPE_SCROLL,
    STYLESHEET_VARIABLE_DTYPE_TRUE,
    STYLESHEET_VARIABLE_DTYPE_FALSE,
    STYLESHEET_VARIABLE_DTYPE_ABSOLUTE,
    STYLESHEET_VARIABLE_DTYPE_RELATIVE,
    STYLESHEET_VARIABLE_DTYPE_NONE,
    STYLESHEET_VARIABLE_DTYPE_INPUT_NUMBER,
    STYLESHEET_VARIABLE_DTYPE_UNKNOWN,
} StylesheetVariableDtype;

typedef union StylesheetVariableValue {
    struct RGB rgb;
    int _int;
    u16 _u16;
    u8  _u8;
} StylesheetVariableValue;

typedef struct StylesheetVariable {
    StylesheetVariableValue value;
} StylesheetVariable;

typedef struct StylesheetVariable
{
    enum StylesheetVariableDtype type;
    enum StylesheetVariableDtype type_DEFAULT;
    int value_DEFAULT;
    int value;
} StylesheetVariable;

typedef struct StylesheetVariableBinding
{
    u16 variableIndex;
    u16 itemIndex;
    u16 size;
    u16 offset; // offset in bytes into the corresponding Stylesheet_Item
} StylesheetVariableBinding;

typedef struct StylesheetVariableOverride
{
    enum StylesheetVariableDtype type_OVERRIDE;
    int value_OVERRIDE;
    u16 variableIndex;
} StylesheetVariableOverride;

typedef struct StylesheetScreenQuery
{
    int overrideArrayPartitionStart;
    int overrideArrayPartitionCount;
    enum NU_Style_Token comparator;
    int screenWidth;
} StylesheetScreenQuery;

typedef struct Stylesheet_Scrollbar_Style
{
    // Bar
    u8 width;
    u8 height;
    bool overlay;

    // Thumb
    u8 thumbMinSize;
    u8 thumbBorderTop, thumbBorderBottom, thumbBorderLeft, thumbBorderRight;
    u8 thumbBorderRadiusTl, thumbBorderRadiusTr, thumbBorderRadiusBl, thumbBorderRadiusBr;
    u8 thumbBackgroundR, thumbBackgroundG, thumbBackgroundB;
    u8 thumbBorderR, thumbBorderG, thumbBorderB;

    // Track
    u8 trackPadTop, trackPadBottom, trackPadLeft, trackPadRight;
    u8 trackBorderTop, trackBorderBottom, trackBorderLeft, trackBorderRight;
    u8 trackBorderRadiusTl, trackBorderRadiusTr, trackBorderRadiusBl, trackBorderRadiusBr;
    u8 trackBackgroundR, trackBackgroundG, trackBackgroundB;
    u8 trackBorderR, trackBorderG, trackBorderB;
} Stylesheet_Scrollbar_Style;

typedef struct Stylesheet
{
    Array items;
    Array pseudoItems;
    Array variables;
    Array variableOverrides;
    Array variableBindings;
    Array screenQueries;
    LinearStringset class_string_set;
    LinearStringset id_string_set;
    Hashmap class_item_hashmap;
    Hashmap id_item_hashmap;
    Hashmap tag_item_hashmap;
    Hashmap tag_pseudo_item_hashmap;
    Hashmap class_pseudo_item_hashmap;
    Hashmap id_pseudo_item_hashmap;
    LinearStringmap fontNameIndexMap;
    Container fonts;
    Stylesheet_Item defaultStyleItem;
    Stylesheet_Scrollbar_Style scrollbarStyle;
} Stylesheet;

typedef struct StyleTextRef
{
    u32 NU_Token_index;
    u32 src_index;
    u8 char_count;
} StyleTextRef;
