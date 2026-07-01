#!/bin/bash

# Paths
srcInclude="src"
sdlLib="src/libraries/sdl3/lib"
sdlInclude="src/libraries/sdl3/include"
glewInclude="src/libraries/glew/include"
glewLib="src/libraries/glew/lib"
freetypeInclude="src/libraries/freetype/include"
freetypeLib="src/libraries/freetype/lib"

# Compile
clang -std=c99 -O3 \
    "src/nodus.c" \
    -I"$srcInclude" \
    -I"$glewInclude" \
    -I"$sdlInclude" \
    -I"$freetypeInclude" \
    -L"$glewLib" \
    -L"$sdlLib" \
    -L"$freetypeLib" \
    -lGLEW -lSDL3 -framework OpenGL -framework Cocoa -lfreetype \
    -shared \
    -o "nodus/mac/lib/nodus.dylib" \
    -Wno-deprecated-declarations

# Remove the .exp file if it exists (equivalent to Windows .exp)
rm -f nodus/mac/lib/nodus.exp

# Copy nodus.h into nodus/mac/include
cp -f src/nodus.h nodus/mac/include/
