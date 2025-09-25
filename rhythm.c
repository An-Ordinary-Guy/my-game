#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define MAX_BULLETS 100
#define MAX_OBSTACLES 50
#define TRACK_WIDTH 20.0f
#define TRACK_LENGTH 200.0f
#define BEAT_INTERVAL 60.0f/140  // Seconds between beats (140 BPM)
#define PLAYER_SPEED 15.0f
#define BULLET_SPEED 25.0f

// Audio variables
Music backgroundMusic;
bool audioInitialized = false;

typedef struct {
    Vector3 position;
    Vector3 velocity;
    bool active;
    float size;
} Bullet;

typedef struct {
    Vector3 position;
    Vector3 size;
    bool active;
    Color color;
} Obstacle;

typedef struct {
    Vector3 position;
    Vector3 size;
    int health;
    float shootTimer;
} Boss;

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float size;
    int health;
    int lane; // 0=left, 1=center, 2=right
} Player;

// Game variables
Player player;
Boss boss;
Bullet bullets[MAX_BULLETS];
Obstacle obstacles[MAX_OBSTACLES];
Camera3D camera = { 0 };
float beatTimer = 0.0f;
float gameTime = 0.0f;
bool gameWon = false;
bool gameOver = false;
int score = 0;
float nextObstacleSpawn = 0.0f;
float lastBeatTime = 0.0f;

// Function declarations
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void SpawnBulletOnBeat(void);
void SpawnObstacle(void);
void UpdatePlayer(void);
void UpdateBullets(void);
void UpdateObstacles(void);
void UpdateBoss(void);
bool CheckCollision3D(Vector3 pos1, float size1, Vector3 pos2, float size2);
void DrawBeatIndicator(void);
bool IsBeatHit(void);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Rhythm Boss Runner");
    InitAudioDevice(); // Initialize audio system
    SetTargetFPS(60);
    
    srand(time(NULL));
    InitGame();
    
    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }
    
    // Clean up audio
    if (audioInitialized) {
        UnloadMusicStream(backgroundMusic);
        CloseAudioDevice();
    }
    
    CloseWindow();
    return 0;
}

void InitGame(void) {
    // Try to initialize audio
    if (!audioInitialized) {
        // Load the background music
        backgroundMusic = LoadMusicStream("assets/StereoMadness.wav");
        if (backgroundMusic.stream.buffer != NULL) {
            PlayMusicStream(backgroundMusic);
        }
        
        audioInitialized = true;
    }
    
    // Initialize player
    player.position = (Vector3){ 0.0f, 1.0f, -180.0f };
    player.velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
    player.size = 2.0f;
    player.health = 5;
    player.lane = 1; // Start in center lane
    
    // Initialize boss
    boss.position = (Vector3){ 0.0f, 8.0f, 180.0f }; // Boss stays at the end
    boss.size = (Vector3){ 6.0f, 8.0f, 4.0f };
    boss.health = 20;
    boss.shootTimer = 0.0f;
    
    // Initialize camera
    camera.position = (Vector3){ 0.0f, 12.0f, player.position.z - 15.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, player.position.z + 5.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    
    // Initialize obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
    }
    
    // Reset timers
    beatTimer = 0.0f;
    gameTime = 0.0f;
    lastBeatTime = 0.0f;
    nextObstacleSpawn = 3.0f;
    gameWon = false;
    gameOver = false;
    score = 0;
}

bool IsBeatHit(void) {
    // Check if we should spawn bullets based on music beat
    // Using simple time-based beat detection for 140 BPM
    float currentTime = GetMusicTimePlayed(backgroundMusic);
    float beatInterval = 60.0f / 140.0f; // 140 BPM
    
    // Find the current beat position
    float beatPosition = fmod(currentTime, beatInterval);
    float timeSinceLastBeat = currentTime - lastBeatTime;
    
    // Only trigger on certain beats (every 2nd beat for example)
    static int beatCount = 0;
    
    // Trigger on beat (within a small window)
    if (timeSinceLastBeat >= beatInterval - 0.05f) {
        lastBeatTime = currentTime;
        beatCount++;
        
        // Only shoot on every 2nd beat (you can adjust this pattern)
        if (beatCount % 2 == 0) {
            return true;
        }
    }
    
    return false;
}

void UpdateGame(void) {
    if (gameOver || gameWon) {
        if (IsKeyPressed(KEY_R)) {
            InitGame();
        }
        return;
    }
    
    // Update music stream
    if (audioInitialized && backgroundMusic.stream.buffer != NULL) {
        UpdateMusicStream(backgroundMusic);
        
        // Restart music if it ends
        if (!IsMusicStreamPlaying(backgroundMusic)) {
            PlayMusicStream(backgroundMusic);
        }
    }
    
    float deltaTime = GetFrameTime();
    gameTime += deltaTime;
    
    // Beat detection using the music
    if (IsBeatHit()) {
        SpawnBulletOnBeat();
        score += 10; // Score for surviving each beat
    }
    
    // Spawn obstacles periodically
    if (gameTime >= nextObstacleSpawn) {
        SpawnObstacle();
        nextObstacleSpawn = gameTime + (2.0f + (rand() % 3)); // 2-4 seconds
    }
    
    UpdatePlayer();
    UpdateBullets();
    UpdateObstacles();
    UpdateBoss();
    
    // Update camera to follow player
    camera.position.z = player.position.z - 15.0f;
    camera.target.z = player.position.z + 5.0f;
    
    // Check win condition (reach the boss)
    if (player.position.z >= boss.position.z - 8.0f) {
        gameWon = true;
    }
    
    // Check game over
    if (player.health <= 0) {
        gameOver = true;
    }
}

void UpdatePlayer(void) {
    float deltaTime = GetFrameTime();
    
    // Lane switching (A/D or Left/Right arrows) - FIXED: A goes right, D goes left
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_RIGHT)) {
        if (player.lane < 2) player.lane++; // A/Right goes to right lane
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT)) {
        if (player.lane > 0) player.lane--; // D/Left goes to left lane
    }
    
    // Set target position based on lane
    float targetX = (player.lane - 1) * 8.0f; // -8, 0, 8 for left, center, right
    
    // Smooth movement to target lane
    float lerpSpeed = 8.0f;
    player.position.x += (targetX - player.position.x) * lerpSpeed * deltaTime;
    
    // Move forward constantly
    player.position.z += PLAYER_SPEED * deltaTime;
    
    // Check collision with bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            if (CheckCollision3D(player.position, player.size, bullets[i].position, bullets[i].size)) {
                player.health--;
                bullets[i].active = false;
                score = fmaxf(0, score - 50); // Lose points for getting hit
            }
        }
    }
    
    // Check collision with obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            Vector3 obstacleCenter = {
                obstacles[i].position.x,
                obstacles[i].position.y + obstacles[i].size.y / 2,
                obstacles[i].position.z
            };
            
            if (CheckCollision3D(player.position, player.size, obstacleCenter, 3.0f)) {
                player.health--;
                obstacles[i].active = false;
                score = fmaxf(0, score - 30); // Lose points for hitting obstacles
            }
        }
    }
}

void UpdateBullets(void) {
    float deltaTime = GetFrameTime();
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].position.x += bullets[i].velocity.x * deltaTime;
            bullets[i].position.y += bullets[i].velocity.y * deltaTime;
            bullets[i].position.z += bullets[i].velocity.z * deltaTime;
            
            // Remove bullets that are too far behind or to the sides
            if (bullets[i].position.z < player.position.z - 30.0f ||
                fabs(bullets[i].position.x) > 25.0f ||
                bullets[i].position.y < -5.0f) {
                bullets[i].active = false;
            }
        }
    }
}

void UpdateObstacles(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            // Remove obstacles that are far behind the player
            if (obstacles[i].position.z < player.position.z - 20.0f) {
                obstacles[i].active = false;
            }
        }
    }
}

void UpdateBoss(void) {
    float deltaTime = GetFrameTime();
    boss.shootTimer += deltaTime;
    
    // Boss is now stationary - no animation
}

void SpawnBulletOnBeat(void) {
    // Spawn 1-3 bullets aimed at different lanes
    int numBullets = 1 + (rand() % 3);
    
    for (int b = 0; b < numBullets; b++) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].position = (Vector3){ 
                    boss.position.x + (rand() % 3 - 1) * 2.0f, 
                    boss.position.y - 2.0f, 
                    boss.position.z 
                };
                
                // Aim at different lanes
                float targetX = (rand() % 3 - 1) * 8.0f; // Random lane
                float targetZ = player.position.z - 10.0f; // Slightly behind player
                
                Vector3 direction = {
                    targetX - bullets[i].position.x,
                    -3.0f, // Slight downward angle
                    targetZ - bullets[i].position.z
                };
                
                // Normalize and scale velocity
                float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
                if (length > 0) {
                    bullets[i].velocity.x = (direction.x / length) * BULLET_SPEED;
                    bullets[i].velocity.y = (direction.y / length) * BULLET_SPEED;
                    bullets[i].velocity.z = (direction.z / length) * BULLET_SPEED;
                }
                
                bullets[i].size = 1.0f;
                bullets[i].active = true;
                break;
            }
        }
    }
}

void SpawnObstacle(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            int lane = rand() % 3;
            float xPos = (lane - 1) * 8.0f;
            
            obstacles[i].position = (Vector3){ xPos, 0.0f, player.position.z + 40.0f + (rand() % 20) };
            obstacles[i].size = (Vector3){ 3.0f, 4.0f, 3.0f };
            obstacles[i].active = true;
            
            // Random obstacle colors
            Color colors[] = { RED, ORANGE, PURPLE, MAROON };
            obstacles[i].color = colors[rand() % 4];
            break;
        }
    }
}

bool CheckCollision3D(Vector3 pos1, float size1, Vector3 pos2, float size2) {
    float distance = sqrtf(
        (pos1.x - pos2.x) * (pos1.x - pos2.x) +
        (pos1.y - pos2.y) * (pos1.y - pos2.y) +
        (pos1.z - pos2.z) * (pos1.z - pos2.z)
    );
    
    return distance < (size1 + size2);
}

void DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    BeginMode3D(camera);
    
    // Draw track/ground
    DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ TRACK_WIDTH * 2, TRACK_LENGTH * 2 }, DARKGRAY);
    
    // Draw lane dividers
    for (int i = -1; i <= 1; i++) {
        float x = i * 8.0f;
        for (float z = player.position.z - 30.0f; z < player.position.z + 50.0f; z += 5.0f) {
            DrawCube((Vector3){ x, 0.1f, z }, 0.2f, 0.2f, 3.0f, YELLOW);
        }
    }
    
    // Draw track boundaries
    for (float z = player.position.z - 30.0f; z < player.position.z + 50.0f; z += 2.0f) {
        DrawCube((Vector3){ -12.0f, 1.0f, z }, 1.0f, 2.0f, 1.0f, GRAY);
        DrawCube((Vector3){ 12.0f, 1.0f, z }, 1.0f, 2.0f, 1.0f, GRAY);
    }
    
    // Draw player
    DrawCube(player.position, player.size, player.size, player.size, BLUE);
    DrawCubeWires(player.position, player.size, player.size, player.size, SKYBLUE);
    
    // Draw boss
    DrawCube(boss.position, boss.size.x, boss.size.y, boss.size.z, RED);
    DrawCubeWires(boss.position, boss.size.x, boss.size.y, boss.size.z, MAROON);
    
    // Draw bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            DrawSphere(bullets[i].position, bullets[i].size, ORANGE);
        }
    }
    
    // Draw obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            DrawCube(obstacles[i].position, obstacles[i].size.x, obstacles[i].size.y, obstacles[i].size.z, obstacles[i].color);
            DrawCubeWires(obstacles[i].position, obstacles[i].size.x, obstacles[i].size.y, obstacles[i].size.z, WHITE);
        }
    }
    
    // Draw distance markers
    for (float z = 0; z <= boss.position.z; z += 20.0f) {
        DrawCube((Vector3){ 15.0f, 2.0f, z }, 0.5f, 4.0f, 0.5f, GREEN);
    }
    
    EndMode3D();
    
    // Draw UI
    DrawText(TextFormat("Health: %d", player.health), 10, 10, 20, WHITE);
    DrawText(TextFormat("Score: %d", score), 10, 35, 20, WHITE);
    DrawText(TextFormat("Distance to Boss: %.1fm", boss.position.z - player.position.z), 10, 60, 20, WHITE);
    
    // Draw beat indicator
    DrawBeatIndicator();
    
    // Controls
    DrawText("A/Right or D/Left: Switch Lanes", 10, SCREEN_HEIGHT - 50, 16, WHITE);
    DrawText("Dodge bullets and obstacles to reach the boss!", 10, SCREEN_HEIGHT - 25, 16, WHITE);
    
    // Game over/win screens
    if (gameOver) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
        DrawText("GAME OVER!", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 40, 40, RED);
        DrawText("Press R to restart", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 20, 20, WHITE);
    } else if (gameWon) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
        DrawText("YOU REACHED THE BOSS!", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 40, 40, GREEN);
        DrawText(TextFormat("Final Score: %d", score), SCREEN_WIDTH/2 - 70, SCREEN_HEIGHT/2, 20, WHITE);
        DrawText("Press R to restart", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 40, 20, WHITE);
    }
    
    EndDrawing();
}

void DrawBeatIndicator(void) {
    // Beat visualization based on music
    float currentTime = GetMusicTimePlayed(backgroundMusic);
    float beatInterval = 60.0f / 140.0f;
    float beatProgress = fmod(currentTime, beatInterval) / beatInterval;
    
    int barWidth = 200;
    int barHeight = 20;
    int barX = SCREEN_WIDTH - barWidth - 20;
    int barY = 20;
    
    DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
    DrawRectangle(barX, barY, (int)(barWidth * beatProgress), barHeight, 
                 beatProgress > 0.8f ? RED : (beatProgress > 0.6f ? YELLOW : GREEN));
    DrawText("BEAT", barX, barY - 25, 20, WHITE);
    
    // Visual beat pulse
    if (beatProgress > 0.9f) {
        DrawCircle(SCREEN_WIDTH - 50, 70, 20.0f * (beatProgress - 0.9f) * 10.0f, Fade(WHITE, 0.5f));
    }
}