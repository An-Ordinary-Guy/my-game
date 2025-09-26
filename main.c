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

int map[100];
void initLevel(){
    for (int i = 0; i < 100; i++){
        map[i] = 1;
    }
}

void buildLevel(){
    for (int i = 0; i < 100; i++){
        if (map[i]){
            DrawRectangle(i*100, 500, 100, 100, RED);
        }
    }
}

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
    player->currentY = GROUND_Y; // Start on ground
}



// Handle input - instantly move to lane and attack
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
                    break; // Only hit one note per input
                }
            }
        }
}



// Draw the player character with texture
void DrawPlayer(Player* player, Texture2D playerTexture) {

    int textureX = PLAYER_X - playerTexture.width / 2 - 100;
    int textureY = player->currentY - playerTexture.height / 2;
   
    DrawTexture(playerTexture, textureX, textureY, WHITE);
}

int main(){
    InitWindow(WindowWidth, WindowHeight, "practice");
    InitAudioDevice();
    InitBeatMap();
    initLevel();

    // Initialize player
    Player player;
    InitPlayer(&player);

    // Load player texture
    Texture2D playerTexture = LoadTexture("assets/capy.png");
    

    Music levelMusic = LoadMusicStream("assets/adofai.mp3");
    bool musicStarted = false;

    timer MusicTimer;
    MusicTimer.active = true;
    MusicTimer.duration = 5.0;
    MusicTimer.repeat = false;
    MusicTimer.startTime = GetTime();
    
    int score = 0;
    double songTime = 0;

    while(!WindowShouldClose()){
        //float deltaTime = GetFrameTime();
        
        

        if (MusicTimer.active && GetTime() - MusicTimer.startTime > MusicTimer.duration){
            PlayMusicStream(levelMusic);
            musicStarted = true;
            if (!MusicTimer.repeat) MusicTimer.active = false;
        }
        
        if (musicStarted){
            UpdateMusicStream(levelMusic);
            songTime = GetMusicTimePlayed(levelMusic);
        }

        if (musicStarted){
            for (int i = 0; i < count; i++){
                float timeUntilHit = beatMap[i].time - songTime;
                if (timeUntilHit <= 2) beatMap[i].active = true;
                if (beatMap[i].active && !beatMap[i].hit){
                    beatMap[i].x = 200 + (timeUntilHit/2)*(WindowWidth-200);
                    if (beatMap[i].x < -10) beatMap[i].active = false;
                } 
            }
        }

        // Handle player input (jump and attack)
        if (musicStarted) {
            HandlePlayerInput(&player, beatMap, count, songTime, &score);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        buildLevel();

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
                    if (beatMap[i].lane == 'l') DrawCircle((int)beatMap[i].x, GROUND_Y, 50, BLUE);
                }
            }
        }

        // Draw lanes
        DrawRectangle(200, AIR_Y, WindowWidth, 2, GRAY);
        DrawRectangle(200, GROUND_Y, WindowWidth, 2, YELLOW);
        DrawLine(200, 0, 200, WindowHeight, RED);

        // Draw hit zones
        DrawCircleLines(PLAYER_X, AIR_Y, 50, GREEN);
        DrawCircleLines(PLAYER_X, GROUND_Y, 50, BLUE);
        
        // Draw the player
        DrawPlayer(&player, playerTexture);
        
        // Draw UI
        DrawText(TextFormat("Score: %d", score), 10, 10, 30, RED);
        DrawText("N: Air Lane", 10, 50, 20, WHITE);
        DrawText("M: Ground Lane", 10, 70, 20, WHITE);
        
        // Debug info
        DrawText(TextFormat("Player Y: %d", player.currentY), 10, 100, 20, WHITE);
        DrawText(TextFormat("Lane: %s", (player.currentY == AIR_Y) ? "AIR" : "GROUND"), 10, 120, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(playerTexture);
    UnloadMusicStream(levelMusic);
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}