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
    uint16_t objectNo;     /**< Unique identifier for the object within its type. */
    uint8_t hp;          /**< Health points of the object. */
    float x;              /**< X coordinate of the object. */
    float y;              /**< Y coordinate of the object. */
} ObjectState;

/**
 * @brief Returns a pointer to the structure representing the local player.
 *
 * This function provides access to the local player's object state (`myPlayer`),
 * which can be read or modified as needed.
 *
 * @return Pointer to the `myPlayer` object.
 */
volatile ObjectState* get_my_player(void);

/**
 * @brief Returns pointer to array of objects for a given type index.
 *
 * @param typeIndex Index of the object type (0 to 3).
 * @return Pointer to the array of ObjectState for that type, or NULL if out of range.
 */
volatile ObjectState* get_objects_lists(int typeIndex);

/**
 * @brief Returns a pointer to the array of object counts for each type.
 *
 * This function returns a pointer to the array (`counts`) holding the number
 * of active objects for each type.
 *
 * @return Pointer to the `counts` array.
 */
volatile size_t* get_object_counts(void);

/**
 * @brief Returns the danger distance for spark objects.
 *
 * This value defines the radius within which sparks are considered dangerous.
 *
 * @return Floating point value representing the danger distance for sparks.
 */
float get_danger_dist_spark();

/**
 * @brief Returns the danger distance for glue hazards.
 *
 * This value defines the radius within which glue objects are considered dangerous.
 *
 * @return Floating point value representing the danger distance for glue.
 */
float get_danger_dist_glue();

/**
 * @brief Returns the danger distance for bigger players.
 *
 * This value defines the radius within which larger players are considered a threat.
 *
 * @return Floating point value representing the danger distance for bigger players.
 */
float get_danger_dist_bigger_plr();

/**
 * @brief Returns the last stored movement or facing angle.
 *
 * The angle is typically used for tracking or adjusting the player's movement direction.
 *
 * @return Last stored angle as a floating point value.
 */
float get_last_angle();

/**
 * @brief Sets and returns the last movement or facing angle.
 *
 * This function updates the stored angle and returns the new value.
 *
 * @param angle New angle value to store.
 * @return The angle that was set.
 */
float set_last_angle(float angle);

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
