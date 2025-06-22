#include "logic.h"
#include "utils.h"
#include "float.h"

void find_closest_static_object(float myX, float myY, volatile ObjectState objects[], size_t count,
                                float* closestObjectDist, float* closestObjectAngle) {
    for (size_t i = 0; i < count; i++) {
        if (objects[i].hp <= 0) continue;

        float dist = compute_distance(myX, myY, objects[i].x, objects[i].y);
        if (dist < *closestObjectDist) {
            *closestObjectDist = dist;
            *closestObjectAngle = compute_angle(myX, myY, objects[i].x, objects[i].y);
        }
    }
}

void find_closest_player(float myX, float myY, uint8_t myHP,
                         volatile ObjectState objects[], size_t count,
                         float* closestBiggerDist, float* closestBiggerAngle,
                         float* closestSmallerDist, float* closestSmallerAngle) {
    for (size_t i = 0; i < count; i++) {
        if (objects[i].hp == 0) continue;

        float dist = compute_distance(myX, myY, objects[i].x, objects[i].y);
        float angle = compute_angle(myX, myY, objects[i].x, objects[i].y);

        if (objects[i].hp > myHP && dist < *closestBiggerDist) {
            *closestBiggerDist = dist;
            *closestBiggerAngle = angle;
        }
        else if (objects[i].hp < myHP && dist < *closestSmallerDist) {
            *closestSmallerDist = dist;
            *closestSmallerAngle = angle;
        }
    }
}

float decide_target_angle(float closestBiggerPlayerDist, float closestBiggerPlayerAngle,
                          float closestSmallerPlayerDist, float closestSmallerPlayerAngle,
                          float closestTransistorDist, float closestTransistorAngle,
                          float closestSparkDist, float closestSparkAngle,
                          float closestGlueDist, float closestGlueAngle) {

    if (closestSparkDist < DANGER_DIST_SPARK)
        return lastAngle = avoid_angle(closestSparkAngle);

    if (closestBiggerPlayerDist < DANGER_DIST_BIGGER)
        return lastAngle = avoid_angle(closestBiggerPlayerAngle);

    // if (closestGlueDist < DANGER_DIST_GLUE)
    //     return lastAngle = avoid_angle(closestGlueAngle);

    float scorePrey = (closestSmallerPlayerDist < FLT_MAX)
        ? attraction_score(closestSmallerPlayerDist, 3.0f) : 0.0f;

    float scoreTransistor = (closestTransistorDist < FLT_MAX)
        ? attraction_score(closestTransistorDist, 5.0f) : 0.0f;

    if (scorePrey >= scoreTransistor && closestSmallerPlayerDist < FLT_MAX)
        return lastAngle = closestSmallerPlayerAngle;

    if (closestTransistorDist < FLT_MAX)
        return lastAngle = closestTransistorAngle;

    return lastAngle = get_random_explore_angle(lastAngle);
}

float compute_move_angle(float myX, float myY, uint8_t myHP) {
    float closestDists[4]  = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    float closestAngles[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    float closestBiggerPlayerDist = FLT_MAX, closestBiggerPlayerAngle = 0.0f;
    float closestSmallerPlayerDist = FLT_MAX, closestSmallerPlayerAngle = 0.0f;

    find_closest_player(myX, myY, myHP,objects[0], counts[0],
                        &closestBiggerPlayerDist, &closestBiggerPlayerAngle,
                        &closestSmallerPlayerDist, &closestSmallerPlayerAngle);

    for (int i = 1; i <= 3; i++) {
        find_closest_static_object(myX, myY, objects[i], counts[i], &closestDists[i], &closestAngles[i]);
    }

    return decide_target_angle(
        closestBiggerPlayerDist, closestBiggerPlayerAngle,
        closestSmallerPlayerDist, closestSmallerPlayerAngle,
        closestDists[1], closestAngles[1],
        closestDists[2], closestAngles[2],
        closestDists[3], closestAngles[3]
    );
}
