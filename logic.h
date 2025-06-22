#ifndef LOGIC_H
#define LOGIC_H

#include "objects.h"
#include <stdint.h>

void find_closest_static_object(float myX, float myY, volatile ObjectState objects[], size_t count,
                                float* closestObjectDist, float* closestObjectAngle);

void find_closest_player(float myX, float myY, int myHP,
                         volatile ObjectState objects[], size_t count,
                         float* closestBiggerDist, float* closestBiggerAngle,
                         float* closestSmallerDist, float* closestSmallerAngle);

float decide_target_angle(float closestBiggerPlayerDist, float closestBiggerPlayerAngle,
                          float closestGlueDist, float closestGlueAngle,
                          float closestSmallerPlayerDist, float closestSmallerPlayerAngle,
                          float closestSparkDist, float closestSparkAngle,
                          float closestTransistorDist, float closestTransistorAngle);

float compute_move_angle(float myX, float myY, uint8_t myHP);

#endif //LOGIC_H
