#pragma once

#include "nu_stylesheet_structs.h"
#include "nu_stylesheet_tokens.h"
void Stylesheet_ApplyScreenQueries(Stylesheet* ss, NodeP* root)
{
    int w, h;
    SDL_Window* rootWin = GetSDL_Window(&GUI.winManager, root->windowID);
    SDL_GetWindowSizeInPixels(rootWin, &w, &h);

    // Apply screen query overrides to variables in order
    for (int i=0; i<ss->screenQueries; i++) {
        StylesheetScreenQuery* query = Array_Get(&ss->screenQueries, i);

        bool applies = false;
        applies |= query->comparator == STYLE_GREATER       && w > query->screenWidth;
        applies |= query->comparator == STYLE_LESS          && w < query->screenWidth;
        applies |= query->comparator == STYLE_GREATER_EQUAL && w >= query->screenWidth;
        applies |= query->comparator == STYLE_LESS_EQUAL    && w <= query->screenWidth;

        if (applies) {
            for (int j=query->overrideArrayPartitionStart;
                j<query->overrideArrayPartitionStart + query->overrideArrayPartitionCount;
                j++)
            {
                StylesheetVariableOverride* override = Array_Get(&ss->variableOverrides, j);
                StylesheetVariable* variableToOverride = Array_Get(&ss->variables, override->variableIndex);
                variableToOverride->type = override->type_OVERRIDE;
                variableToOverride->value = override->value_OVERRIDE;
            }
        }
    }

    // Apply variable values to stylesheet items
    for (int i=0; i<ss->variableBindings.size; i++)
    {
        StylesheetVariableBinding* binding = Array_Get(&ss->variableBindings, i);
        StylesheetVariable* variable = Array_Get(&ss->variables, binding->variableIndex);
        Stylesheet_Item* item = Array_Get(&ss->items, binding->itemIndex);

        switch (variable->type) {
            case STYLESHEET_VARIABLE_DTYPE_NUMBER
        }
    }


    BreadthFirstSearch_Reset(&GUI.bfs, root);
    NodeP* node;
    while (BreadthFirstSearch_Next(&GUI.bfs, &node)) {



    }
}
