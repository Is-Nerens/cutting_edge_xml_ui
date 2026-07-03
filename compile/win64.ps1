$srcInclude = "src"
$sdlLib = "src\libraries\sdl3\win64"
$sdlInclude = "src\libraries\sdl3\include"
$glewInclude = "src\libraries\glew\include"
$glewLib = "src\libraries\glew\win64"
$freetypeInclude = "src\libraries\freetype\include"
$freetypeLib = "src\libraries\freetype\win64"
clang -std=c99 -O3 -fopenmp "src\nodus.c" `
-I"$srcInclude" `
-I"$glewInclude" `
-I"$sdlInclude" `
-I"$freetypeInclude" `
-L"$glewLib" `
-L"$sdlLib" `
-L"$freetypeLib" `
-lglew32 -lSDL3 -lopengl32 -lgdi32 -lfreetype -ladvapi32 `
"-Wl,/SUBSYSTEM:WINDOWS" `
-shared `
-o "nodus\win64\nodus.dll" -Wno-deprecated-declarations
Remove-Item nodus\win64\nodus.exp -Force

# Copy nodus.h into nodus/win64/include
New-Item -ItemType Directory -Force -Path "nodus/include" | Out-Null
Copy-Item -Force "src/nodus.h" "nodus/include/
