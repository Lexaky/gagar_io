#include "raylib.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "DBmethods.h"
#include "GameMethods.h"

using namespace std;

// ����������� �������� ������ + �����
const int screenWidth = 800;
const int screenHeight = 600;
const int mapWidth = 2000;
const int mapHeight = 2000;

// ��������� ����
const int enemyCount = 16; // ���������� ������
const int initialMass = 10; // ��������� �����
const float initialSpeed = 200.0f; //��������� ��������
const int foodCount = 100; // ���������� ��� �� ����� (��� �������� ���������� � ��. �����)
const int minFoodVisible = 10; // ����� ����� ������� 10 ������ ��� ������ ����
const float visionFactor = 10.0f; // ������� ��������� ��� �����
const float massDecayRate = 0.5f; // ����� ����� ������� � ������ �������� (�������� �� ��������� �����)
const bool isGameContinueAfterDeath = false; // ����������� �� ���� � ���� �����, ����� ����� ����
// --> ����������� ���� � ������ --> �� �� ����� ������� ���������
// false �� ����.

// ������� ������ � ���
std::vector<Circle> enemies;
std::vector<Vector2> foods;

// ��� ������ �����
template<typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int main() {
    InitWindow(screenWidth, screenHeight, "GAGAGAGAGAGAGAGAGARIO");
    SetTargetFPS(60);

    Circle player = { { mapWidth / 2.0f, mapHeight / 2.0f }, initialMass, BLACK };
    //����� ��������� � ������ ����� ������
    //���� �� ������ � ������ 255?
    initEnemies();
    initFoods();

    // ������ ��� ���������� �� �������
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    bool gameOver = false;

    bool isBestShowed = false;
    int bestResult = 0;

    bool isLastShowed = false;
    int lastResult = 0;

    while (!WindowShouldClose()) {
        if (!isBestShowed)
        {
            bestResult = getMaxScore();
            isBestShowed = true;
        }

        if (!isLastShowed)
        {
            lastResult = getLastSavedScoreFromDB();
            isLastShowed = true;
        }

        if (gameOver) {
            // ����� ��������� � ����� ���� � ����������� �����������
            if (IsKeyPressed(KEY_R)) {
                // ���������� ���� ��� ������� R
                player.position = { mapWidth / 2.0f, mapHeight / 2.0f };
                player.mass = initialMass;
                if (!isGameContinueAfterDeath)
                    initEnemies();
                initFoods();
                gameOver = false;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("END GAME; 'R' - RESTART",
                screenWidth / 2 - MeasureText("END GAME; 'R' - RESTART", 20) / 2,
                screenHeight / 2, 20,
                BLACK);
            EndDrawing();
            continue;
        }

        float playerSpeedActual = calculateSpeed(player.mass) * GetFrameTime();


        // �������� ������ �� �������
        if (IsKeyDown(KEY_W)) player.position.y -= playerSpeedActual;
        if (IsKeyDown(KEY_S)) player.position.y += playerSpeedActual;
        if (IsKeyDown(KEY_A)) player.position.x -= playerSpeedActual;
        if (IsKeyDown(KEY_D)) player.position.x += playerSpeedActual;

        // ���� ����� �� ������� �� ������� ����� ���������� ��� ������� � ������ �������
        float playerRadius = calculateRadius(player.mass);
        player.position.x = clamp(player.position.x, playerRadius, (float)mapWidth - playerRadius);
        player.position.y = clamp(player.position.y, playerRadius, (float)mapHeight - playerRadius);

        // ���������� ��������� ���� ������
        for (Circle& enemy : enemies) {
            updateEnemyBehavior(enemy, player, enemies, foods);
            updateMassDecay(enemy);
        }

        // ����� ���������� � ����
        for (Vector2& food : foods) {
            if (isColliding(player, food)) {
                player.mass += 1; // ���������� ����� ������
                food = { (float)(rand() % mapWidth), 
                    (float)(rand() % mapHeight) }; // ��������� ��� � ��������� ����� �����
            }
        }

        // ����������, �� � �������
        for (Circle& enemy : enemies) {
            for (Vector2& food : foods) {
                if (isColliding(enemy, food)) {
                    enemy.mass += 1; 
                    food = { (float)(rand() % mapWidth), 
                        (float)(rand() % mapHeight) };
                }
            }
        }

        // ����� ���������� � ������ (�������� �.�. �� ���� �����������, �.�. �� ����� �� ���)
        for (Circle& enemy : enemies) {
            if (isColliding(player, enemy)) {
                if (player.mass > enemy.mass) {
                    player.mass += enemy.mass; //����� �������� ������
                    enemy.position = { (float)(rand() % mapWidth), (float)(rand() % mapHeight) }; // ����������� ����� � ��������� (*) �����
                    enemy.mass = initialMass; // ���� ��� �������� ������ ���
                }
                else { //����� ������ ����� --> ��������� ������ �� ����
                    saveScoreToDB(player.mass);
                    isLastShowed = false;
                    isBestShowed = false;
                    gameOver = true; // ��������� ���������� ���� ���������
                }
            }
        }

        // ��������� ������������ ����� ����� �������
        for (Circle& enemy1 : enemies) {
            for (Circle& enemy2 : enemies) {
                if (&enemy1 != &enemy2 && isColliding(enemy1, enemy2)) {
                    if (enemy1.mass > enemy2.mass) { // �������� �� ��, ��� ������
                        enemy1.mass += enemy2.mass; // ����� ������� �������� ������
                        enemy2.position = { (float)(rand() % mapWidth), 
                            (float)(rand() % mapHeight) }; // ����������� ����� � ��������� (*)
                        enemy2.mass = initialMass; // ����� ����� ���������� �����
                    }
                    else if (enemy2.mass > enemy1.mass) { // ����������
                        enemy2.mass += enemy1.mass;
                        enemy1.position = { (float)(rand() % mapWidth), (float)(rand() % mapHeight) }; // ����������� ����� � ��������� (*)
                        enemy1.mass = initialMass;
                    }
                }
            }
        }

        // ���������� ����� ������ �� ��������
        updateMassDecay(player);

        // ������� ������ �� ����� ��� ������ ������ ����
        camera.target = player.position;

        // ���������:
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera); //����� ��� ������

        // 1) �����
        DrawRectangle(0, 0, mapWidth, mapHeight, LIGHTGRAY);

        // 2) ������
        DrawCircleV(player.position, calculateRadius(player.mass), player.color);

        // 3) ������
        for (const Circle& enemy : enemies) {
            DrawCircleV(enemy.position, calculateRadius(enemy.mass), enemy.color);
        }

        // 4) ���
        for (const Vector2& food : foods) {
            DrawCircleV(food, 5, GREEN);
        }

        EndMode2D();

        // 5) ����� �����
        DrawText(TextFormat("Mass: %i", (int)player.mass), 10, 10, 20, BLACK);

        // ��������� ����� ���, ���� ������� ��� ������ ������������
        // ���� ������ ��� ��������� ����� ���, � ����� ����� �� ����� ���������� ��� �� �����
        Vector2 topLeft = GetScreenToWorld2D({ 0, 0 }, camera);
        Vector2 bottomRight = GetScreenToWorld2D({ (float)screenWidth, (float)screenHeight }, camera);
        int visibleFoods = std::count_if(foods.begin(), foods.end(), [&](Vector2 food) {
            return food.x >= topLeft.x && food.x <= bottomRight.x && food.y >= topLeft.y && food.y <= bottomRight.y;
            });

        while (visibleFoods < minFoodVisible) {
            foods.push_back({ (float)(rand() % mapWidth), (float)(rand() % mapHeight) });
            visibleFoods++;
        }
        
        // ������ �� ������� 
        drawScore(bestResult);
        // ���� ���������
        drawLastScore(lastResult, true);

        // ����� ���������
        EndDrawing();
    }

    CloseWindow();
    return 0;
}