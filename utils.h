#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Computes the Euclidean distance between two 2D points.
 *
 * @param x1 X-coordinate of the first point.
 * @param y1 Y-coordinate of the first point.
 * @param x2 X-coordinate of the second point.
 * @param y2 Y-coordinate of the second point.
 * @return Distance between the two points.
 */
float compute_distance(float x1, float y1, float x2, float y2);

/**
 * @brief Computes the angle in radians from (x1, y1) to (x2, y2).
 *
 * The angle is measured in standard polar coordinates using atan2.
 *
 * @param x1 X-coordinate of the source point.
 * @param y1 Y-coordinate of the source point.
 * @param x2 X-coordinate of the destination point.
 * @param y2 Y-coordinate of the destination point.
 * @return Angle in radians in the range [-π, π].
 */
float compute_angle(float x1, float y1, float x2, float y2);

/**
 * @brief Returns the angle opposite to the given angle.
 *
 * Useful for reversing direction.
 *
 * @param angle Input angle in radians.
 * @return Reversed angle normalized to [-π, π].
 */
float reverse_angle(float angle);

/**
 * @brief Calculates an attraction score based on distance and weight.
 *
 * Higher weight and lower distance produce a higher score.
 *
 * @param distance Distance to the target.
 * @param weight Importance or desirability of the target.
 * @return Computed score.
 */
float attraction_score(float distance, float weight);

/**
 * @brief Normalizes an angle to the range [0, 2π).
 *
 * Ensures angle is positive and within a single circle revolution.
 *
 * @param angle Angle in radians.
 * @return Normalized angle in the range [0, 2π).
 */
float normalize_angle(float angle);

/**
 * @brief Generates a slightly random angle around the last known direction.
 *
 * Used to explore with some noise/randomness.
 *
 * @param last Last known angle.
 * @return New angle with small random variation, normalized to [0, 2π).
 */
float get_random_explore_angle(float last);

/**
 * @brief Computes an escape angle away from a threat.
 *
 * Chooses randomly between left, right, or back relative to the threat angle.
 *
 * @param threatAngle Angle to the threatening object.
 * @return Escape angle in radians, normalized to [0, 2π).
 */
float avoid_angle(float threatAngle);

#endif //UTILS_H
