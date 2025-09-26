#include "include/raylib.h"
#include "include/raymath.h"
#include <stdio.h>






int main() {
    // Window
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Muse Dash Jump Example");

    // Player properties
    Rectangle player = {100, screenHeight - 100, 50, 50}; // x, y, width, height
    float velocityY = 0.0f;
    const float gravity = 1000.0f;
    const float jumpForce = -500.0f;
    bool isOnGround = true;

    // Ground
    Rectangle ground = {0, screenHeight - 50, screenWidth, 50};

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Jump input
        if (IsKeyPressed(KEY_SPACE) && isOnGround) {
            velocityY = jumpForce;
            isOnGround = false;
        }

        // Apply gravity
        velocityY += gravity * dt;
        player.y += velocityY * dt;

        // Check collision with ground
        if (player.y + player.height >= ground.y) {
            player.y = ground.y - player.height;
            velocityY = 0;
            isOnGround = true;
        }

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangleRec(ground, DARKGRAY);
        DrawRectangleRec(player, BLUE);

        DrawText("Press SPACE to jump", 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
