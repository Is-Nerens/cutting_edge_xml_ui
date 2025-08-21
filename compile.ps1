$sdlLib = "lib\SDL3\lib\x64" 
$sdlInclude = "lib\SDL3\include"
$SDL_ttf_Lib = "lib\SDL3_ttf\lib\x64"
$SDL_ttf_Include = "lib\SDL3_ttf\include"
$glewInclude = "lib\glew\include"
$glewLib = "lib\glew\lib\x64"
$nanovgInclude = "lib\nanoVG"

# Use nanovg.c instead of nanovg_gl.c
clang -std=c99 main.c "lib\nanoVG\nanovg.c" -I"$glewInclude" -I"$sdlInclude" -I"$SDL_ttf_Include" -I"$nanovgInclude" -L"$glewLib" -L"$sdlLib" -L"$SDL_ttf_Lib" -lglew32 -lSDL3 -lSDL3_ttf -lopengl32 -lgdi32 -o app.exe -Wno-deprecated-declarations

