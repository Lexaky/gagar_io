#include "raylib.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "DBmethods.h"
#include "GameMethods.h"

using namespace std;

// Определение размеров экрана + карты
const int screenWidth = 800;
const int screenHeight = 600;
const int mapWidth = 2000;
const int mapHeight = 2000;

// Настройка игры
const int enemyCount = 16; // Количество врагов
const int initialMass = 10; // Начальная масса
const float initialSpeed = 200.0f; //Начальная скорость
const int foodCount = 100; // Количество еды по карте (при съедании появляется в др. месте)
const int minFoodVisible = 10; // Игрок видит минимум 10 единиц еды вокруг себя
const float visionFactor = 10.0f; // Область видимости для врага
const float massDecayRate = 0.5f; // Какая масса убывает с каждой секундой (убывание до начальной массы)
const bool isGameContinueAfterDeath = false; // Продолжится ли игра с того места, когда игрок умер
// --> Сохраняются веса у врагов --> он же новый уровень сложности
// false не сохр.

// Векторы врагов и еды
std::vector<Circle> enemies;
std::vector<Vector2> foods;

// Для границ карты
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
    //Игрок спавнится в центре карты всегда
    //Цвет мб рандом с альфой 255?
    initEnemies();
    initFoods();

    // Камера для следования за игроком
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
            // Вывод сообщения о конце игры и возможности перезапуска
            if (IsKeyPressed(KEY_R)) {
                // Перезапуск игры при нажатии R
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


        // Движение игрока по нажатию
        if (IsKeyDown(KEY_W)) player.position.y -= playerSpeedActual;
        if (IsKeyDown(KEY_S)) player.position.y += playerSpeedActual;
        if (IsKeyDown(KEY_A)) player.position.x -= playerSpeedActual;
        if (IsKeyDown(KEY_D)) player.position.x += playerSpeedActual;

        // Чтоб игрок не выходил за пределы карты пересчитал его позицию в случае попытки
        float playerRadius = calculateRadius(player.mass);
        player.position.x = clamp(player.position.x, playerRadius, (float)mapWidth - playerRadius);
        player.position.y = clamp(player.position.y, playerRadius, (float)mapHeight - playerRadius);

        // Обновление поведения всех врагов
        for (Circle& enemy : enemies) {
            updateEnemyBehavior(enemy, player, enemies, foods);
            updateMassDecay(enemy);
        }

        // Игрок встретился с едой
        for (Vector2& food : foods) {
            if (isColliding(player, food)) {
                player.mass += 1; // Увеличение массы игрока
                food = { (float)(rand() % mapWidth), 
                    (float)(rand() % mapHeight) }; // Генерация еды в рандомной точке карты
            }
        }

        // Аналогично, но с врагами
        for (Circle& enemy : enemies) {
            for (Vector2& food : foods) {
                if (isColliding(enemy, food)) {
                    enemy.mass += 1; 
                    food = { (float)(rand() % mapWidth), 
                        (float)(rand() % mapHeight) };
                }
            }
        }

        // Игрок встретился с врагом (проверка д.б. на всех противников, т.к. хз какой из них)
        for (Circle& enemy : enemies) {
            if (isColliding(player, enemy)) {
                if (player.mass > enemy.mass) {
                    player.mass += enemy.mass; //Игрок оказался больше
                    enemy.position = { (float)(rand() % mapWidth), (float)(rand() % mapHeight) }; // Перемещение врага в случайную (*) карты
                    enemy.mass = initialMass; // Враг при респавне теряет вес
                }
                else { //Игрок меньше врага --> обработка выхода из игры
                    saveScoreToDB(player.mass);
                    isLastShowed = false;
                    isBestShowed = false;
                    gameOver = true; // Обработка завершения игры завершена
                }
            }
        }

        // Обработка столкновений между всеми врагами
        for (Circle& enemy1 : enemies) {
            for (Circle& enemy2 : enemies) {
                if (&enemy1 != &enemy2 && isColliding(enemy1, enemy2)) {
                    if (enemy1.mass > enemy2.mass) { // Проверка на то, кто больше
                        enemy1.mass += enemy2.mass; // Масса перешла большому ботяре
                        enemy2.position = { (float)(rand() % mapWidth), 
                            (float)(rand() % mapHeight) }; // Перемещение врага в случайную (*)
                        enemy2.mass = initialMass; // Сброс массы съеденного врага
                    }
                    else if (enemy2.mass > enemy1.mass) { // Аналогично
                        enemy2.mass += enemy1.mass;
                        enemy1.position = { (float)(rand() % mapWidth), (float)(rand() % mapHeight) }; // Перемещение врага в случайную (*)
                        enemy1.mass = initialMass;
                    }
                }
            }
        }

        // Уменьшение массы игрока со временем
        updateMassDecay(player);

        // наводка камеры на новую поз игрока каждый кадр
        camera.target = player.position;

        // Отрисовка:
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera); //Нужно для камеры

        // 1) карты
        DrawRectangle(0, 0, mapWidth, mapHeight, LIGHTGRAY);

        // 2) игрока
        DrawCircleV(player.position, calculateRadius(player.mass), player.color);

        // 3) врагов
        for (const Circle& enemy : enemies) {
            DrawCircleV(enemy.position, calculateRadius(enemy.mass), enemy.color);
        }

        // 4) еды
        for (const Vector2& food : foods) {
            DrawCircleV(food, 5, GREEN);
        }

        EndMode2D();

        // 5) Текст массы
        DrawText(TextFormat("Mass: %i", (int)player.mass), 10, 10, 20, BLACK);

        // Генерация новой еды, если видимой еды меньше минимального
        // надо просто для генерации новой еды, и чтобы игрок не искал оставшуюся еду по карте
        Vector2 topLeft = GetScreenToWorld2D({ 0, 0 }, camera);
        Vector2 bottomRight = GetScreenToWorld2D({ (float)screenWidth, (float)screenHeight }, camera);
        int visibleFoods = std::count_if(foods.begin(), foods.end(), [&](Vector2 food) {
            return food.x >= topLeft.x && food.x <= bottomRight.x && food.y >= topLeft.y && food.y <= bottomRight.y;
            });

        while (visibleFoods < minFoodVisible) {
            foods.push_back({ (float)(rand() % mapWidth), (float)(rand() % mapHeight) });
            visibleFoods++;
        }
        
        // рекорд за сегодня 
        drawScore(bestResult);
        // ласт результат
        drawLastScore(lastResult, true);

        // конец отрисовки
        EndDrawing();
    }

    CloseWindow();
    return 0;
}