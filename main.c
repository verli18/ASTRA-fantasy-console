//gcc main.c -l raylib -lm -Wall -Wextra -g 
//^compile command because I have a phobia of makefiles
#include "raylib.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int screenWidth = 320*3;
    int screenHeight = 240*3;
    InitWindow(screenWidth, screenHeight, "emulator");
    #include "rendering/rendering.c"
    VDP.isFirstCommand = true;

    SetTargetFPS(30);  
    while(!WindowShouldClose()){ 
        Vector2 mousePos = GetMousePosition();
        mousePos.x = mousePos.x/3;
        mousePos.y = 240-(mousePos.y/3);
        // if(IsKeyDown(KEY_A)) {
        //     quad.a = mousePos;
        // }
        // if(IsKeyDown(KEY_B)) {
        //     quad.b = mousePos;
        // }
        // if(IsKeyDown(KEY_C)) {
        //     quad.c = mousePos;
        // }
        // if(IsKeyDown(KEY_D)) {
        //     quad.d = mousePos;
        // }

        // if(IsKeyPressed(KEY_Q)) {
        //     shading.type -= 1;
        // }
        // if(IsKeyPressed(KEY_W)) {
        //     shading.type += 1;
        // }

        drawScreen(&VDP);
        
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureEx(vScreen.texture, (Vector2){0.0f,0.0f}, 0.0f, 3.0f, WHITE);
        EndDrawing();
    }
    CloseWindow();
    free(VDP.vramPointer);  
    return 0;
}