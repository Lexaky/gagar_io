#include "GameMethods.h"

const int screenWidth = 800;
const int screenHeight = 600;
const int mapWidth = 2000;
const int mapHeight = 2000;

const int enemyCount = 16; // Количество врагов
const int initialMass = 10; // Начальная масса
const float initialSpeed = 200.0f; //Начальная скорость
const int foodCount = 100; // Количество еды по карте (при съедании появляется в др. месте)
const int minFoodVisible = 10; // Игрок видит минимум 10 единиц еды вокруг себя
const float visionFactor = 10.0f; // Область видимости для врага
const float massDecayRate = 0.5f; // Какая масса убывает с каждой секундой (убывание до начальной массы)
const bool isGameContinueAfterDeath = false; // Продолжится ли игра с того места, когда игрок умер


extern std::vector<Circle> enemies;
extern std::vector<Vector2> foods;

// Шаблонная функция для ограничения значения value между min и max
template<typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float calculateRadius(float mass) {
    return sqrt(mass) * 5; //Начальная масса 10 -> радиус круга ~ 7 ед.
}

// Функция для вычисления скорости круга в зависимости от массы
float calculateSpeed(float mass) {
    return initialSpeed / (1.0f + mass / 150.0f);
}

// Инициализация врагов на карте
void initEnemies() {
    enemies.clear(); // Очистка списка врагов
    // Создание заданного количества врагов с случайными параметрами
    for (int i = 0; i < enemyCount; i++) {
        Circle enemy;
        enemy.position = { (float)(rand() % mapWidth), (float)(rand() % mapHeight) }; // Кинул в случайную поз.
        //Мб проверка на то, что противник при респавне не попадёт в позицию игрока?
        enemy.mass = initialMass; // Начальная масса

        //raylib color - {char r, char g, char b, char alpha (a)}
        enemy.color = {
            static_cast<unsigned char>(rand() % 256),
            static_cast<unsigned char>(rand() % 256),
            static_cast<unsigned char>(rand() % 256),
            255
        }; // Случайный полный цвет
        enemies.push_back(enemy);
    }
}

// Инициализация еды на карте
void initFoods() {
    foods.clear();

    // Создание заданного количества еды с случайными позициями
    for (int i = 0; i < foodCount; i++) {
        foods.push_back({ (float)(rand() % mapWidth),
            (float)(rand() % mapHeight) }); // Добавление случайной позиции еды
    }
}

// Функция для вычисления расстояния между точками a и b
float distance(Vector2 a, Vector2 b) {
    // x^2 - y^2 = r^2
    // r = sqrt(x^2 - y^2
    return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

//// Функция для проверки столкновения между двумя кругами
//bool isColliding(Circle a, Circle b) {
//    //Придумать что-то разумное, а не просто касание врага врагом или игрока врагом,
//    //круг b залезает в область круга a - тогда коллизия есть
//    
//    float dist = distance(a.position, b.position);
//    return dist < (calculateRadius(a.mass) + calculateRadius(b.mass));
//}
// Решения:
//1) Рассчёт пересекаемой площади двух объектов и сравнение насколько площадь съедаемого
//   в процентах поглащена (концепция и расчёты на листке)
//2) Рассчёт дистанций (концепция и расчёты на листке)

// Функция для проверки столкновения между двумя кругами
bool isColliding(Circle a, Circle b) {

    float dist = distance(a.position, b.position);
    if ((dist <= calculateRadius(b.mass) && b.mass > a.mass) ||
        (dist <= calculateRadius(a.mass) && a.mass > b.mass))
        return true;
    else return false;
}

// Функция для проверки столкновения круга с едой (по сути с точкой центра еды)
bool isColliding(Circle a, Vector2 b) {
    float dist = distance(a.position, b);
    return dist < calculateRadius(a.mass);
}

// Функция для перемещения круга в направлении от точки target с заданной скоростью
void moveCircleAwayFrom(Circle& circle, Vector2 target, float speed) {
    if (circle.position.x < target.x) circle.position.x -= speed;
    if (circle.position.x > target.x) circle.position.x += speed;
    if (circle.position.y < target.y) circle.position.y -= speed;
    if (circle.position.y > target.y) circle.position.y += speed;
}

// в направлении к точке -
// инверсия moveCircleAwayFrom
void moveCircleTowards(Circle& circle, Vector2 target, float speed) {
    if (circle.position.x < target.x) circle.position.x += speed;
    if (circle.position.x > target.x) circle.position.x -= speed;
    if (circle.position.y < target.y) circle.position.y += speed;
    if (circle.position.y > target.y) circle.position.y -= speed;
}

// Обновление поведения врага в зависимости от положения игрока, других врагов и еды
void updateEnemyBehavior(Circle& enemy, const Circle& player, const std::vector<Circle>& enemies, const std::vector<Vector2>& foods) {
    float enemySpeedActual = calculateSpeed(enemy.mass) * GetFrameTime(); // Вычисление актуальной скорости врага
    //Умн на время, проедшее с ласт кадра для плавности

    float visiondistance = calculateRadius(player.mass) * visionFactor; // Вычисление дистанции видимости врага

    // Проверка видимости игрока
    bool seesPlayer = distance(enemy.position, player.position) < visiondistance;

    if (seesPlayer) {
        if (enemy.mass > player.mass) {
            moveCircleTowards(enemy, player.position, enemySpeedActual); // Движение в направлении игрока
        }
        else {
            moveCircleAwayFrom(enemy, player.position, enemySpeedActual); // Отдаление от игрока
        }
    }
    else {
        // Проверка видимости других врагов
        bool seesOtherEnemy = false;
        for (const auto& otherEnemy : enemies) {
            if (&enemy != &otherEnemy) {
                float otherEnemydistance = distance(enemy.position, otherEnemy.position);
                if (otherEnemydistance < visiondistance) {
                    seesOtherEnemy = true;
                    if (enemy.mass > otherEnemy.mass) {
                        moveCircleTowards(enemy, otherEnemy.position, enemySpeedActual); // Движение в направлении другого врага
                    }
                    else {
                        moveCircleAwayFrom(enemy, otherEnemy.position, enemySpeedActual); // Отдаление от другого врага
                    }
                    break;
                }
            }
        }

        if (!seesOtherEnemy) {
            // Движение в направлении ближайшей еды, если других врагов не видно
            float closestdistance = INFINITY;
            Vector2 closestFood = { 0, 0 };
            for (const auto& food : foods) {
                float fooddistance = distance(enemy.position, food);
                if (fooddistance < closestdistance) {
                    closestdistance = fooddistance;
                    closestFood = food;
                }
            }
            moveCircleTowards(enemy, closestFood, enemySpeedActual);
        }
    }

    // Ограничение движения в карте (аналогична для игрока в main)
    float enemyRadius = calculateRadius(enemy.mass);
    enemy.position.x = clamp(enemy.position.x, enemyRadius, (float)mapWidth - enemyRadius);
    enemy.position.y = clamp(enemy.position.y, enemyRadius, (float)mapHeight - enemyRadius);
}

// уменьшение массы круга со временем (до начальной)
void updateMassDecay(Circle& circle) {
    if (circle.mass > initialMass) {
        circle.mass -= massDecayRate * GetFrameTime();
        if (circle.mass < initialMass) {
            circle.mass = initialMass;
        }
    }
}

//Доп методы к БД-шным запросам для отображения результатов

// Отрисовка счёта для best score
void drawScore(int score)
{
    DrawText(TextFormat("Today best score: %d", score), 10, 40, 20, DARKGRAY);
}
// Отрисовка счёта для ластового score
void drawLastScore(int score, bool lastScore)
{
    DrawText(TextFormat("Last score: %d", score), 10, 60, 20, DARKGRAY);
}
