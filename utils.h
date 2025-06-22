#ifndef UTILS_H
#define UTILS_H

float compute_distance(float x1, float y1, float x2, float y2);

float compute_angle(float x1, float y1, float x2, float y2);

float reverse_angle(float angle);

float attraction_score(float distance, float weight);

float normalize_angle(float angle);

float get_random_explore_angle(float last);

float avoid_angle(float threatAngle);

#endif //UTILS_H
