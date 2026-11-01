#ifndef VHS_COLLISION_H
#define VHS_COLLISION_H
#include "world.h"
#include <stdint.h>
typedef struct {
    uint32_t *sum;
    int width, height, room;
    bool ready;
} CollisionMap;
bool collisionLoad(CollisionMap *m, const char *path);
bool collisionLoadRoom(CollisionMap *m, int room);
void collisionFree(CollisionMap *m);
bool collisionRect(const CollisionMap *m, Rectangle r);
bool collisionFeet(const CollisionMap *m, Vector2 p);
Vector2 collisionMove(const CollisionMap *m, Vector2 p, Vector2 delta);
#endif
