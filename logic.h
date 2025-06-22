#ifndef LOGIC_H
#define LOGIC_H

#include "objects.h"
#include <stdint.h>

/**
 * @brief Finds the closest static object (e.g. transistor, spark, glue) to the player.
 *
 * @param myX Player's current X coordinate.
 * @param myY Player's current Y coordinate.
 * @param objects Array of objects of a specific type.
 * @param count Number of objects in the array.
 * @param closestObjectDist Pointer to the variable where the closest distance will be stored (updated if closer found).
 * @param closestObjectAngle Pointer to the variable where the angle to the closest object will be stored.
 */
void find_closest_static_object(float myX, float myY, volatile ObjectState objects[], size_t count,
                                float* closestObjectDist, float* closestObjectAngle);

/**
 * @brief Finds the closest player that is either stronger or weaker than the current player.
 *
 * @param myX Player's current X coordinate.
 * @param myY Player's current Y coordinate.
 * @param myHP Player's current health points.
 * @param objects Array of other player objects.
 * @param count Number of players in the array.
 * @param closestBiggerDist Pointer to store distance to the closest stronger player (HP > myHP).
 * @param closestBiggerAngle Pointer to store angle to the closest stronger player.
 * @param closestSmallerDist Pointer to store distance to the closest weaker player (HP < myHP).
 * @param closestSmallerAngle Pointer to store angle to the closest weaker player.
 */
void find_closest_player(float myX, float myY, uint8_t myHP,
                         volatile ObjectState objects[], size_t count,
                         float* closestBiggerDist, float* closestBiggerAngle,
                         float* closestSmallerDist, float* closestSmallerAngle);

/**
 * @brief Decides in which direction the player should move based on surroundings.
 *
 * @param closestBiggerPlayerDist Distance to the nearest stronger player.
 * @param closestBiggerPlayerAngle Angle to the nearest stronger player.
 * @param closestGlueDist Distance to the nearest glue hazard.
 * @param closestGlueAngle Angle to the nearest glue hazard.
 * @param closestSmallerPlayerDist Distance to the nearest weaker player.
 * @param closestSmallerPlayerAngle Angle to the nearest weaker player.
 * @param closestSparkDist Distance to the nearest spark hazard.
 * @param closestSparkAngle Angle to the nearest spark hazard.
 * @param closestTransistorDist Distance to the nearest transistor.
 * @param closestTransistorAngle Angle to the nearest transistor.
 * @return Angle in radians indicating the best movement direction.
 */
float decide_target_angle(float closestBiggerPlayerDist, float closestBiggerPlayerAngle,
                          float closestGlueDist, float closestGlueAngle,
                          float closestSmallerPlayerDist, float closestSmallerPlayerAngle,
                          float closestSparkDist, float closestSparkAngle,
                          float closestTransistorDist, float closestTransistorAngle);

/**
 * @brief Computes the best angle to move based on the current game state.
 *
 * @param myX Player's current X coordinate.
 * @param myY Player's current Y coordinate.
 * @param myHP Player's current health points.
 * @return Angle in radians for the direction the player should move.
 */
float compute_move_angle(float myX, float myY, uint8_t myHP);

#endif // LOGIC_H
