#include "include/raylib.h"
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>


const int WindowWidth = 1200;
const int WindowHeight = 800;

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



Notes beatMap[150];
int count = 38;

void InitBeatMap(){
    FILE *src = fopen("assets/test_song_beatmap.csv", "r");
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



int main(){

    InitWindow(WindowWidth, WindowHeight, "practice");
    InitAudioDevice();
    InitBeatMap();
    initLevel();

    Music levelMusic = LoadMusicStream("assets/test_song.wav");
    bool musicStarted = false;

    timer MusicTimer;
    MusicTimer.active = true;
    MusicTimer.duration = 5.0;
    MusicTimer.repeat = false;
    MusicTimer.startTime = GetTime();
    
    
    
    
    int score = 0;

    double songTime = 0;

    while(!WindowShouldClose()){


        if (MusicTimer.active && GetTime() - MusicTimer.startTime > MusicTimer.duration){
            PlayMusicStream(levelMusic);
            musicStarted = true;
            if (!MusicTimer.repeat) MusicTimer.active = false;
        }
        if (musicStarted){
            UpdateMusicStream(levelMusic);
            songTime = GetMusicTimePlayed(levelMusic);
        }
        
        
        //printf("%f, %f\n", songTime, GetTime());
        
        //float dt = GetFrameTime();

        if (musicStarted){
            for (int i = 0; i < count; i++){
                //printf("%lf,%c\n", beatMap[i].time, beatMap[i].lane);
            float timeUntilHit = beatMap[i].time - songTime;
                if (timeUntilHit  <= 2) beatMap[i].active = true;
                if (beatMap[i].active && !beatMap[i].hit){
                    beatMap[i].x = 200 + (timeUntilHit/2)*(WindowWidth-200);

                    if (beatMap[i].x < -10) beatMap[i].active = false;
                } 
            }
        }
        
        

        int h_input = (int)IsKeyPressed(KEY_N);
        int l_input = (int)IsKeyPressed(KEY_M);

        if (musicStarted){
            for (int i = 0; i < count; i++){
                if(h_input && beatMap[i].lane == 'h' || l_input && beatMap[i].lane == 'l'){
                    
                    float diff = fabs(beatMap[i].time - songTime);
                    if (diff <= 0.15){
                        beatMap[i].hit = true;
                        beatMap[i].active = false;  
                        score += 10;
                    }
                    
                }
            
            }
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
                    if (beatMap[i].lane == 'h') DrawCircle((int)beatMap[i].x, 200, 50, GREEN);
                    if (beatMap[i].lane == 'l') DrawCircle((int)beatMap[i].x, 400, 50, BLUE);
                }
            }
        }
        

        DrawRectangle(200, 200,WindowWidth, 2, GRAY);
        DrawRectangle(200, 400,WindowWidth, 2, YELLOW);
        DrawLine(200,0,200,WindowHeight, RED);

        DrawCircleLines(200,200,50,GREEN);
        DrawCircleLines(200,400,50, BLUE);
        DrawText(TextFormat("%d", score), 0, 0, 50, RED);
        

        EndDrawing();
        

    }

    UnloadMusicStream(levelMusic);
    CloseAudioDevice();
    CloseWindow();
    

    return 0;
}