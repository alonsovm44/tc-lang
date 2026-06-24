# Generate C code
./tigc game.tc -o game.c

# Compile with raylib
gcc game.c -o app.exe -Isrc -IC:/raylib/raylib/src -LC:/raylib/raylib/build/raylib  -lraylib -lws2_32 -lwinhttp -lopengl32 -lgdi32 -lwinmm