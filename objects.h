#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdint.h>

/**
 * @brief Maximum number of objects per type.
 */
#define MAX_OBJECTS_TYPE 100

/**
 * @brief Size in bytes of a single object record in the payload.
 */
#define RECORD_SIZE 12

/**
 * @brief Structure representing the state of an object in the game world.
 */
typedef struct {
    uint8_t objectType;   /**< Type of the object (e.g., player, item, hazard). */
    uint8_t objectNo;     /**< Unique identifier for the object within its type. */
    uint16_t hp;          /**< Health points of the object. */
    float x;              /**< X coordinate of the object. */
    float y;              /**< Y coordinate of the object. */
} ObjectState;

/**
 * @brief The state of the local player.
 */
extern volatile ObjectState myPlayer;

/**
 * @brief All visible objects, categorized by type.
 *
 * Indexes:
 * - 0: Players
 * - 1: Transistors
 * - 2: Sparks (dangerous)
 * - 3: Glue (dangerous)
 */
extern volatile ObjectState objects[4][MAX_OBJECTS_TYPE];

/**
 * @brief Count of objects for each type in the objects array.
 */
extern volatile size_t counts[4];

/**
 * @brief Minimum distance at which sparks are considered dangerous.
 */
extern const float DANGER_DIST_SPARK;

/**
 * @brief Minimum distance at which glue puddles are considered dangerous.
 */
extern const float DANGER_DIST_GLUE;

/**
 * @brief Minimum distance at which larger players are considered dangerous.
 */
extern const float DANGER_DIST_BIGGER;

/**
 * @brief Last angle returned by the movement computation logic.
 */
extern float lastAngle;

/**
 * @brief Processes an incoming object update payload.
 *
 * This function parses the binary payload, updates the list of known objects,
 * and modifies the local player state if needed.
 *
 * @param payload Pointer to the raw data buffer.
 * @param payload_len Length of the payload in bytes (must be a multiple of RECORD_SIZE).
 */
void handle_object_update_request(const uint8_t *payload, size_t payload_len);

/**
 * @brief Updates or removes an object from the list based on its state.
 *
 * If an object with the same number already exists, it will be updated or removed
 * if its HP is zero. If it is a new object with non-zero HP, it will be added.
 *
 * @param list Pointer to the array of objects of a specific type.
 * @param count Pointer to the number of objects currently in the list.
 * @param newObj Pointer to the object state to insert or update.
 */
void update_object_list(volatile ObjectState *list, volatile size_t *count, const ObjectState *newObj);

#endif // OBJECTS_H
