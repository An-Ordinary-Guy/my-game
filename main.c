#include "include/raylib.h"
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

const int WindowWidth = 1200;
const int WindowHeight = 800;

// Player position constants
const int PLAYER_X = 200;
const int GROUND_Y = 400;
const int AIR_Y = 200;

// Player state
typedef struct {
    int currentY;
} Player;

typedef struct{
    float duration;
    float startTime;
    bool repeat;
    bool active;
}timer;

typedef struct Notes{
    double time;
    char lane;
    bool hit;
    bool active;
    float x;
}Notes;

// int map[100];
// void initLevel(){
//     for (int i = 0; i < 100; i++){
//         map[i] = 1;
//     }
// }

// void buildLevel(){
//     for (int i = 0; i < 100; i++){
//         if (map[i]){
//             DrawRectangle(i*100, 500, 100, 100, RED);
//         }
//     }
// }

Notes beatMap[151];
int count = 151;

void InitBeatMap(){
    FILE *src = fopen("assets/Labels1.csv", "r");
    double time;
    char lane;

    for (int i = 0; i < count; i++){
        fscanf(src, "%lf,%c", &time, &lane);
        beatMap[i].time = time;
        beatMap[i].lane = lane;
        beatMap[i].hit = false;
        beatMap[i].active = false;
        beatMap[i].x = WindowWidth;
    }
    fclose(src);
}

// Initialize player
void InitPlayer(Player* player) {
    player->currentY = GROUND_Y; 
}



void InitNotes(double songTime){
    for (int i = 0; i < count; i++){
        float timeUntilHit = beatMap[i].time - songTime;
        if (timeUntilHit <= 1.5) beatMap[i].active = true;
            if (beatMap[i].active && !beatMap[i].hit){
                beatMap[i].x = 200 + (timeUntilHit/1.5)*(WindowWidth-200);
                    if (beatMap[i].x < -10) beatMap[i].active = false;
            } 
        }
}
void HandlePlayerInput(Player* player, Notes* beatMap, int count, double songTime, int* score) {
    int n_pressed = IsKeyPressed(KEY_N);
    int m_pressed = IsKeyPressed(KEY_M);
    
    if (n_pressed) {
        player->currentY = AIR_Y;     
    }
    
    if (m_pressed) {
        player->currentY = GROUND_Y; 
    }

    for (int i = 0; i < count; i++) {
            if (((beatMap[i].lane == 'h' && n_pressed) || (beatMap[i].lane == 'l' && m_pressed)) && !beatMap[i].hit && beatMap[i].active) {
                float diff = fabs(beatMap[i].time - songTime);
                if (diff <= 0.15) {
                    beatMap[i].hit = true;
                    beatMap[i].active = false;
                    *score += 10;
                    break;
                }
            }
        }
}




void DrawPlayer(Player* player, Texture2D playerTexture) {

    int textureX = PLAYER_X - playerTexture.width / 2 - 100;
    int textureY = player->currentY - playerTexture.height / 2;
   
    DrawTexture(playerTexture, textureX, textureY, WHITE);
}


typedef enum GameState{GamePlay, GameOver}GameState;

int main(){
    InitWindow(WindowWidth, WindowHeight, "practice");
    InitAudioDevice();
    InitBeatMap();

    // Initialize player
    Player player;
    InitPlayer(&player);

    // texture
    Texture2D playerTexture = LoadTexture("assets/capy.png");
    Texture2D bg = LoadTexture("assets/bg.jpg");
    
    //sound
    Music levelMusic = LoadMusicStream("assets/adofai.mp3");
    bool musicStarted = false;


    timer MusicTimer;
    MusicTimer.active = true;
    MusicTimer.duration = 3.0;
    MusicTimer.repeat = false;
    MusicTimer.startTime = GetTime();
    
    int score = 0;
    double songTime = 0;

    GameState gameState = GamePlay;

    while(!WindowShouldClose()){
        //float deltaTime = GetFrameTime();
        
        switch (gameState) {
            case GamePlay:
                if (MusicTimer.active && GetTime() - MusicTimer.startTime > MusicTimer.duration){
                    PlayMusicStream(levelMusic);
                    musicStarted = true;
                    if (!MusicTimer.repeat) MusicTimer.active = false;
                }

                if (musicStarted){
                    UpdateMusicStream(levelMusic);
                    songTime = GetMusicTimePlayed(levelMusic);

                    if (songTime >= GetMusicTimeLength(levelMusic) -){
                        gameState = GameOver;
                        StopMusicStream(levelMusic);
                        break;  
                    }

                    InitNotes(songTime);
                    HandlePlayerInput(&player, beatMap, count, songTime, &score);
                }
                break;

            case GameOver:
                if (IsKeyPressed(KEY_SPACE)) {
                    gameState = GamePlay;
                    score = 0;
                    songTime = 0;
                    musicStarted = false;
                    MusicTimer.startTime = GetTime();
                    MusicTimer.active = true;
                    InitPlayer(&player);

                    for (int i = 0; i < count; i++) {
                        beatMap[i].hit = false;
                        beatMap[i].active = false;
                        beatMap[i].x = WindowWidth;
                    }
                }
                break;
}

        
        

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(bg, 0,-100,WHITE);

        if (gameState == GamePlay){
            if (!musicStarted) {
            int timeLeft = MusicTimer.duration - (GetTime() - MusicTimer.startTime);
            if (timeLeft > 0) {
                DrawText(TextFormat("Starting in %d", timeLeft), WindowWidth/2 - 100, WindowHeight/2, 40, RAYWHITE);
            }
        }

            if (musicStarted){
                for (int i = 0; i < count; i++){
                    if (beatMap[i].active && !beatMap[i].hit){
                        if (beatMap[i].lane == 'h') DrawCircle((int)beatMap[i].x, AIR_Y, 50, GREEN);
                        if (beatMap[i].lane == 'l') DrawCircle((int)beatMap[i].x, GROUND_Y, 50, PURPLE);
                    }
                }
            }

            // lanes
            DrawRectangle(200, AIR_Y, WindowWidth, 2, GRAY);
            DrawRectangle(200, GROUND_Y, WindowWidth, 2, YELLOW);
            //DrawLine(200, 0, 200, WindowHeight, RED);

            // hit zones
            DrawCircleLines(PLAYER_X, AIR_Y, 50, BLACK);
            DrawCircleLines(PLAYER_X, GROUND_Y, 50, BLACK);
            
            
            DrawPlayer(&player, playerTexture);
            
            
            DrawText(TextFormat("Score: %d", score), 10, 10, 30, RED);
            DrawText("N: Air Lane", 10, 50, 20, WHITE);
            DrawText("M: Ground Lane", 10, 70, 20, WHITE);
            
            
        }

        else if (gameState == GameOver){
            if (score >= 1500) {
                DrawText("YOU WIN!", WindowWidth/2 - 120, WindowHeight/2 - 100, 60, GREEN);
            } else {
                DrawText("YOU LOSE!", WindowWidth/2 - 130, WindowHeight/2 - 100, 60, RED);
            }
            DrawText(TextFormat("Final Score: %d", score), WindowWidth/2 - 100, WindowHeight/2 - 20, 30, WHITE);
            DrawText("Press SPACE to restart", WindowWidth/2 - 130, WindowHeight/2 + 20, 25, GRAY);
        }


        
        EndDrawing();
    }

    UnloadTexture(playerTexture);
    UnloadMusicStream(levelMusic);
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}