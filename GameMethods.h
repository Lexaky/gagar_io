#include <iostream>
#include <vector>
#include "Structs.h"

float calculateRadius(float mass);
float calculateSpeed(float mass);
void initEnemies();
void initFoods();
float distance(Vector2 a, Vector2 b);
bool isColliding(Circle a, Circle b);
bool isColliding(Circle a, Vector2 b);
void moveCircleAwayFrom(Circle& circle, Vector2 target, float speed);
void moveCircleTowards(Circle& circle, Vector2 target, float speed);
void updateEnemyBehavior(Circle& enemy, const Circle& player, const std::vector<Circle>& enemies, const std::vector<Vector2>& foods);
void updateMassDecay(Circle& circle);
void drawScore(int score);
void drawLastScore(int score, bool lastScore);