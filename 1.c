#include "include/raylib.h"
#include "include/raymath.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sprite.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define MAX_METEOR 100

typedef struct{
    float duration;
    float startTime;
    bool active;
    bool repeat;
}timer;


void Draw();

int main(){

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Aimation");

    //ship 
    Texture2D spaceship = LoadTexture("assets/spaceship.png");
    Vector2 spaceshipDirection = {0,0};
    Vector2 spaceshipPos = {WINDOW_WIDTH/2,WINDOW_HEIGHT-spaceship.height-20};
    float spaceshipSpeed = 350.0f;
   
    //meteor
    Texture2D meteor[MAX_METEOR];

    // for (int i = 0; i < MAX_METEOR; i++){
    //     meteor[i] = LoadTexture("assets/meteor(1).png");
    //     meteor[i].width = GetRandomValue(50,150);
    //     meteor[i].height = GetRandomValue(50, 150);
    // }

    
   
    Vector2 meteorPos[MAX_METEOR];
    Vector2 meteorDir = {0,1};
    float meteroSpeed[MAX_METEOR];
    int meteorIdx = -1;

    //meteor fall timer
    timer meteorFall = {1.5f, GetTime(), true, true};

    while(!WindowShouldClose()){

        //meteor
        // if (meteorFall.active && GetTime() - meteorFall.startTime > meteorFall.duration){
        //     meteorIdx++;

        //     meteorPos[meteorIdx].x = GetRandomValue(0, WINDOW_WIDTH - meteor[meteorIdx].width);
        //     meteorPos[meteorIdx].y = GetRandomValue(-5, -1);
        //     meteroSpeed[meteorIdx] = GetRandomValue(100, 250);

        //     if (meteorFall.repeat) meteorFall.startTime = GetTime();
        //     else meteorFall.active = false;

           
        // }

        //border collision
        if (spaceshipPos.x + spaceship.width > WINDOW_WIDTH) spaceshipPos.x =  WINDOW_WIDTH - spaceship.width;
        if (spaceshipPos.y + spaceship.height > WINDOW_HEIGHT) spaceshipPos.y =  WINDOW_HEIGHT - spaceship.height;
        if (spaceshipPos.x < 0) spaceshipPos.x = 0;
        if (spaceshipPos.y < 0) spaceshipPos.y = 0;

        float dt = GetFrameTime();

        //Input for spaceship
        spaceshipDirection.x = (int)IsKeyDown(KEY_D) - (int)IsKeyDown(KEY_A);
        spaceshipDirection.y = (int)IsKeyDown(KEY_S) - (int)IsKeyDown(KEY_W);
        spaceshipDirection = Vector2Normalize(spaceshipDirection);

        //movement
        spaceshipPos.x += spaceshipDirection.x * dt * spaceshipSpeed;
        spaceshipPos.y += spaceshipDirection.y * dt * spaceshipSpeed;

        

        Draw();
    }

    CloseWindow();
    UnloadTexture(spaceship);
    

    return 0;
}

void Draw(){
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureV(spaceship,spaceshipPos, WHITE);
        //DrawTextureV(meteor[meteorIdx], meteorPos[meteorIdx], WHITE);


    EndDrawing();
}