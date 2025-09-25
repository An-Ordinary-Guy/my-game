#include "include/raylib.h"
#include <stdlib.h>
#include <stdbool.h>

const int WindowWidth = 1200;
const int WindowHeight = 800;

Rectangle sprite = {200, 400, 80, 120};
Color spriteColor[] = {RED, GREEN, BLUE, ORANGE};

typedef struct{
    float duration;
    float startTime;
    bool repeat;
    bool active;
}timer;

int main(){

    InitWindow(WindowWidth, WindowHeight, "Timer");

    int color = GetRandomValue(0,3);

    timer positionChange;
    positionChange.duration = 1.5f;
    positionChange.startTime = GetTime();
    positionChange.active = true;
    positionChange.repeat = true;

    timer colorChange;
    colorChange.duration = 4.0f;
    colorChange.startTime = GetTime();
    colorChange.active = true;
    colorChange.repeat = false;


    while(!WindowShouldClose()){
        //printf("%lf\n", GetTime());

        //updates
        if (positionChange.active && GetTime() - positionChange.startTime > positionChange.duration){
            //implement at-least once
            sprite.x = GetRandomValue(0, WindowWidth-sprite.width);
            sprite.y = GetRandomValue(0, WindowHeight-sprite.height);
            if (!positionChange.repeat) positionChange.active = false;
            //reset reference
            positionChange.startTime = GetTime();
        }

        if (colorChange.active && GetTime() - colorChange.startTime > colorChange.duration){

            color = GetRandomValue(0,3);

            if (!colorChange.repeat) colorChange.active = false;
            colorChange.startTime = GetTime();
        }
        

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleRec(sprite, spriteColor[color]);

        EndDrawing();


    }

    CloseWindow();

    return 0;
}