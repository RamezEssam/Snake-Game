@ECHO OFF
gcc snake.c -o Snake -O1 -Wall -std=c99 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm