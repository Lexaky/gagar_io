#include "GameMethods.h"

const int screenWidth = 800;
const int screenHeight = 600;
const int mapWidth = 2000;
const int mapHeight = 2000;

const int enemyCount = 16; // ���������� ������
const int initialMass = 10; // ��������� �����
const float initialSpeed = 200.0f; //��������� ��������
const int foodCount = 100; // ���������� ��� �� ����� (��� �������� ���������� � ��. �����)
const int minFoodVisible = 10; // ����� ����� ������� 10 ������ ��� ������ ����
const float visionFactor = 10.0f; // ������� ��������� ��� �����
const float massDecayRate = 0.5f; // ����� ����� ������� � ������ �������� (�������� �� ��������� �����)
const bool isGameContinueAfterDeath = false; // ����������� �� ���� � ���� �����, ����� ����� ����


extern std::vector<Circle> enemies;
extern std::vector<Vector2> foods;

// ��������� ������� ��� ����������� �������� value ����� min � max
template<typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float calculateRadius(float mass) {
    return sqrt(mass) * 5; //��������� ����� 10 -> ������ ����� ~ 7 ��.
}

// ������� ��� ���������� �������� ����� � ����������� �� �����
float calculateSpeed(float mass) {
    return initialSpeed / (1.0f + mass / 150.0f);
}

// ������������� ������ �� �����
void initEnemies() {
    enemies.clear(); // ������� ������ ������
    // �������� ��������� ���������� ������ � ���������� �����������
    for (int i = 0; i < enemyCount; i++) {
        Circle enemy;
        enemy.position = { (float)(rand() % mapWidth), (float)(rand() % mapHeight) }; // ����� � ��������� ���.
        //�� �������� �� ��, ��� ��������� ��� �������� �� ������ � ������� ������?
        enemy.mass = initialMass; // ��������� �����

        //raylib color - {char r, char g, char b, char alpha (a)}
        enemy.color = {
            static_cast<unsigned char>(rand() % 256),
            static_cast<unsigned char>(rand() % 256),
            static_cast<unsigned char>(rand() % 256),
            255
        }; // ��������� ������ ����
        enemies.push_back(enemy);
    }
}

// ������������� ��� �� �����
void initFoods() {
    foods.clear();

    // �������� ��������� ���������� ��� � ���������� ���������
    for (int i = 0; i < foodCount; i++) {
        foods.push_back({ (float)(rand() % mapWidth),
            (float)(rand() % mapHeight) }); // ���������� ��������� ������� ���
    }
}

// ������� ��� ���������� ���������� ����� ������� a � b
float distance(Vector2 a, Vector2 b) {
    // x^2 - y^2 = r^2
    // r = sqrt(x^2 - y^2
    return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

//// ������� ��� �������� ������������ ����� ����� �������
//bool isColliding(Circle a, Circle b) {
//    //��������� ���-�� ��������, � �� ������ ������� ����� ������ ��� ������ ������,
//    //���� b �������� � ������� ����� a - ����� �������� ����
//    
//    float dist = distance(a.position, b.position);
//    return dist < (calculateRadius(a.mass) + calculateRadius(b.mass));
//}
// �������:
//1) ������� ������������ ������� ���� �������� � ��������� ��������� ������� ����������
//   � ��������� ��������� (��������� � ������� �� ������)
//2) ������� ��������� (��������� � ������� �� ������)

// ������� ��� �������� ������������ ����� ����� �������
bool isColliding(Circle a, Circle b) {

    float dist = distance(a.position, b.position);
    if ((dist <= calculateRadius(b.mass) && b.mass > a.mass) ||
        (dist <= calculateRadius(a.mass) && a.mass > b.mass))
        return true;
    else return false;
}

// ������� ��� �������� ������������ ����� � ���� (�� ���� � ������ ������ ���)
bool isColliding(Circle a, Vector2 b) {
    float dist = distance(a.position, b);
    return dist < calculateRadius(a.mass);
}

// ������� ��� ����������� ����� � ����������� �� ����� target � �������� ���������
void moveCircleAwayFrom(Circle& circle, Vector2 target, float speed) {
    if (circle.position.x < target.x) circle.position.x -= speed;
    if (circle.position.x > target.x) circle.position.x += speed;
    if (circle.position.y < target.y) circle.position.y -= speed;
    if (circle.position.y > target.y) circle.position.y += speed;
}

// � ����������� � ����� -
// �������� moveCircleAwayFrom
void moveCircleTowards(Circle& circle, Vector2 target, float speed) {
    if (circle.position.x < target.x) circle.position.x += speed;
    if (circle.position.x > target.x) circle.position.x -= speed;
    if (circle.position.y < target.y) circle.position.y += speed;
    if (circle.position.y > target.y) circle.position.y -= speed;
}

// ���������� ��������� ����� � ����������� �� ��������� ������, ������ ������ � ���
void updateEnemyBehavior(Circle& enemy, const Circle& player, const std::vector<Circle>& enemies, const std::vector<Vector2>& foods) {
    float enemySpeedActual = calculateSpeed(enemy.mass) * GetFrameTime(); // ���������� ���������� �������� �����
    //��� �� �����, �������� � ���� ����� ��� ���������

    float visiondistance = calculateRadius(player.mass) * visionFactor; // ���������� ��������� ��������� �����

    // �������� ��������� ������
    bool seesPlayer = distance(enemy.position, player.position) < visiondistance;

    if (seesPlayer) {
        if (enemy.mass > player.mass) {
            moveCircleTowards(enemy, player.position, enemySpeedActual); // �������� � ����������� ������
        }
        else {
            moveCircleAwayFrom(enemy, player.position, enemySpeedActual); // ��������� �� ������
        }
    }
    else {
        // �������� ��������� ������ ������
        bool seesOtherEnemy = false;
        for (const auto& otherEnemy : enemies) {
            if (&enemy != &otherEnemy) {
                float otherEnemydistance = distance(enemy.position, otherEnemy.position);
                if (otherEnemydistance < visiondistance) {
                    seesOtherEnemy = true;
                    if (enemy.mass > otherEnemy.mass) {
                        moveCircleTowards(enemy, otherEnemy.position, enemySpeedActual); // �������� � ����������� ������� �����
                    }
                    else {
                        moveCircleAwayFrom(enemy, otherEnemy.position, enemySpeedActual); // ��������� �� ������� �����
                    }
                    break;
                }
            }
        }

        if (!seesOtherEnemy) {
            // �������� � ����������� ��������� ���, ���� ������ ������ �� �����
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

    // ����������� �������� � ����� (���������� ��� ������ � main)
    float enemyRadius = calculateRadius(enemy.mass);
    enemy.position.x = clamp(enemy.position.x, enemyRadius, (float)mapWidth - enemyRadius);
    enemy.position.y = clamp(enemy.position.y, enemyRadius, (float)mapHeight - enemyRadius);
}

// ���������� ����� ����� �� �������� (�� ���������)
void updateMassDecay(Circle& circle) {
    if (circle.mass > initialMass) {
        circle.mass -= massDecayRate * GetFrameTime();
        if (circle.mass < initialMass) {
            circle.mass = initialMass;
        }
    }
}

//��� ������ � ��-���� �������� ��� ����������� �����������

// ��������� ����� ��� best score
void drawScore(int score)
{
    DrawText(TextFormat("Today best score: %d", score), 10, 40, 20, DARKGRAY);
}
// ��������� ����� ��� ��������� score
void drawLastScore(int score, bool lastScore)
{
    DrawText(TextFormat("Last score: %d", score), 10, 60, 20, DARKGRAY);
}
