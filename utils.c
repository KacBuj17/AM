#include "utils.h"
#include "math.h"
#include <stdlib.h>

float compute_distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

float compute_angle(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

float reverse_angle(float angle) {
    angle += (float)M_PI;
    if (angle > (float)M_PI) angle -= 2.0f * (float)M_PI;
    return angle;
}

float attraction_score(float distance, float weight) {
    return weight / (distance + 0.01f);
}

float normalize_angle(float angle) {
    while (angle < 0.0f) angle += 2 * M_PI;
    while (angle >= 2 * M_PI) angle -= 2 * M_PI;
    return angle;
}

float get_random_explore_angle(float last) {
    float randomNoise = ((rand() % 1000) / 1000.0f - 0.5f) * 0.6f;
    return normalize_angle(last + randomNoise);
}

float avoid_angle(float threatAngle) {
    float left = normalize_angle(threatAngle + M_PI / 2);
    float right = normalize_angle(threatAngle - M_PI / 2);
    float back = normalize_angle(threatAngle + M_PI);

    int r = rand() % 3;
    if (r == 0) return left;
    if (r == 1) return right;
    return back;
}

