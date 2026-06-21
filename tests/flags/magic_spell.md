# Generate C code
./tigc your_game.tc -o game.c

# Compile with raylib
gcc tests/flags/ray.c -o rayapp.exe -Isrc -IC:/raylib/raylib/src -LC:/raylib/raylib/build/raylib  -lraylib -lws2_32 -lwinhttp -lopengl32 -lgdi32 -lwinmm