$sdlLib = "lib\SDL3\lib\x64" 
$sdlInclude = "lib\SDL3\include"
$SDL_ttf_Lib = "lib\SDL3_ttf\lib\x64"
$SDL_ttf_Include = "lib\SDL3_ttf\include"

clang -std=c99 main.c -I"$sdlInclude" -I"$SDL_ttf_Include" -L"$sdlLib" -L"$SDL_ttf_Lib" -lSDL3 -lSDL3_ttf -o app.exe
