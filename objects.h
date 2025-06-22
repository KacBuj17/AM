#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdint.h>

#define MAX_OBJECTS_TYPE 100
#define RECORD_SIZE 12

typedef struct {
    uint8_t objectType;
    uint8_t objectNo;
    uint16_t hp;
    float x;
    float y;
} ObjectState;

extern volatile ObjectState myPlayer;
extern volatile ObjectState objects[4][MAX_OBJECTS_TYPE];
extern volatile size_t counts[4];

extern const float DANGER_DIST_SPARK;
extern const float DANGER_DIST_GLUE;
extern const float DANGER_DIST_BIGGER;

extern float lastAngle;

void handle_object_update_request(const uint8_t *payload, size_t payload_len);
void update_object_list(volatile ObjectState *list, volatile size_t *count, const ObjectState *newObj);

#endif // OBJECTS_H
