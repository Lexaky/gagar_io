#pragma once
#include "raylib.h"
// Структура для представления игрока, врагов
struct Circle {
    Vector2 position; // Позиция объекта
    float mass;       // Масса объекта
    Color color;      // Цвет объекта
};