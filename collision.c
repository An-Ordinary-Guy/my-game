#include <stdio.h>
#include "include/raylib.h"
#include <math.h>

const int WindowWidth = 1200;
const int WindowHeight = 800;

Rectangle player = {400, 300, 80,80};
float playerSpeed = 300.0f;
Vector2 playerDirection = {0,0};
Vector2 gravity = {0,1};

//map
int levelMap[][19] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

Rectangle blocks[250];
int blockSize = 100;
int blkIdx = 0;

void buildLevel(){
    for (int i = 0; i < 11; i++){
        for (int j = 0; j < 19; j++){
            if (levelMap[i][j]){
                blocks[blkIdx++] = (Rectangle){j*blockSize, i*blockSize, blockSize, blockSize};  
            }
        }
    }
}


//

void collision(char axis){

    for (int i = 0; i < blkIdx; i++){
        if (CheckCollisionRecs(blocks[i], player)){
            if (axis == 'x'){
                if (playerDirection.x > 0) {
                    player.x = blocks[i].x - player.width;
                }else{
                    player.x = blocks[i].x + blocks[i].width;
                }
                
            }
            if (axis == 'y'){
                if (playerDirection.y > 0){
                    player.y = blocks[i].y - player.height;
                }else{
                    player.y = blocks[i].y + blocks[i].height;
                }
            }
        }
    }
}

int main(){

    InitWindow(WindowWidth, WindowHeight, "Collision");

    

    Camera2D camera;
    camera.target = (Vector2) {player.x, player.y};
    camera.offset = (Vector2) {WindowWidth/2, WindowHeight/2};
    camera.zoom = 1;
    buildLevel();
    while(!WindowShouldClose()){

        float dt = GetFrameTime();
        //input
        playerDirection.x = (int)IsKeyDown(KEY_D) - (int)IsKeyDown(KEY_A);
        playerDirection.y = (int)IsKeyDown(KEY_S) - (int)IsKeyDown(KEY_W);
        
        //movement
        player.y += gravity.y * dt * 200.0;
        collision('y');
        player.x += playerDirection.x * dt * playerSpeed;
        collision('x');
        player.y += playerDirection.y * dt * playerSpeed;
        collision('y');

        //camera
        camera.target = (Vector2){player.x, player.y};
        camera.zoom = expf(logf(camera.zoom) + GetMouseWheelMove()*0.1);

        BeginDrawing();
        BeginMode2D(camera);
        ClearBackground(BLACK);
        for (int i = 0; i < blkIdx; i++) DrawRectangleRec(blocks[i], GRAY);
        DrawRectangleRec(player, RED);
        EndMode2D();
        EndDrawing();
    }


    CloseWindow();


    return 0;
}