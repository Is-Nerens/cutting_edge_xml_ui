$sdlLib = "lib\SDL3\lib\x64" 
$sdlInclude = "lib\SDL3\include"

clang -std=c99 main.c -I"$sdlInclude" -L"$sdlLib" -lSDL3 -o app.exe
