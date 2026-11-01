#include "collision.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
bool collisionLoad(CollisionMap *m, const char *path) {
    memset(m, 0, sizeof *m);
    m->room = -1;
    Image im = LoadImage(path);
    if (!im.data)
        return false;
    if (im.width != WORLD_W || im.height != WORLD_H) {
        UnloadImage(im);
        return false;
    }
    Color *p = LoadImageColors(im);
    if (!p) {
        UnloadImage(im);
        return false;
    }
    m->width = im.width;
    m->height = im.height;
    m->sum = calloc((size_t)(m->width + 1) * (m->height + 1), sizeof *m->sum);
    if (!m->sum) {
        UnloadImageColors(p);
        UnloadImage(im);
        return false;
    }
    // Summed-area table built once. Every pixel with alpha > 0 blocks movement.
    for (int y = 0; y < m->height; y++) {
        uint32_t row = 0;
        for (int x = 0; x < m->width; x++) {
            row += p[y * m->width + x].a != 0;
            m->sum[(y + 1) * (m->width + 1) + x + 1] = row + m->sum[y * (m->width + 1) + x + 1];
        }
    }
    UnloadImageColors(p);
    UnloadImage(im);
    m->ready = true;
    return true;
}
bool collisionLoadRoom(CollisionMap *m, int room) {
    if (room < 0 || room >= NUM_CENARIOS)
        return false;
    if (!collisionLoad(m, rooms[room].mask))
        return false;
    m->room = room;
    return true;
}
static bool crossesDoorSeal(const CollisionMap *m, Rectangle r) {
    Vector2 corners[4] = {
        {r.x, r.y}, {r.x + r.width, r.y}, {r.x + r.width, r.y + r.height}, {r.x, r.y + r.height}};
    for (int i = 0; i < doorCount; i++) {
        const Door *d = &doors[i];
        if (d->from != m->room || (d->sealA.x == 0 && d->sealB.x == 0))
            continue;
        if (CheckCollisionPointRec(d->sealA, r) || CheckCollisionPointRec(d->sealB, r))
            return true;
        for (int j = 0; j < 4; j++)
            if (CheckCollisionLines(d->sealA, d->sealB, corners[j], corners[(j + 1) % 4], NULL))
                return true;
    }
    return false;
}
void collisionFree(CollisionMap *m) {
    free(m->sum);
    memset(m, 0, sizeof *m);
}
bool collisionRect(const CollisionMap *m, Rectangle r) {
    if (!isfinite(r.x) || !isfinite(r.y) || !isfinite(r.width) || !isfinite(r.height) || r.width <= 0 ||
        r.height <= 0)
        return true;
    if (r.x < 0 || r.y < 0 || r.x + r.width > WORLD_W || r.y + r.height > WORLD_H)
        return true;
    int x0 = (int)floorf(r.x), y0 = (int)floorf(r.y);
    int x1 = (int)ceilf(r.x + r.width), y1 = (int)ceilf(r.y + r.height);
    if (x0 < 0 || y0 < 0 || x1 > WORLD_W || y1 > WORLD_H)
        return true;
    if (!m->ready)
        return true;
    // Masks keep doorway openings; portal thresholds stop movement beyond the
    // scene until outward movement triggers the transition. Teammates' PNGs are unchanged.
    if (crossesDoorSeal(m, r))
        return true;
    int s = m->width + 1;
    return m->sum[y1 * s + x1] - m->sum[y0 * s + x1] - m->sum[y1 * s + x0] + m->sum[y0 * s + x0] > 0;
}
bool collisionFeet(const CollisionMap *m, Vector2 p) { return collisionRect(m, feet(p)); }
Vector2 collisionMove(const CollisionMap *m, Vector2 p, Vector2 d) {
    if (!isfinite(d.x) || !isfinite(d.y) || collisionFeet(m, p))
        return p;
    // At most one source-image pixel per substep: closed one-pixel contours cannot
    // be tunneled through, even after a long frame. Resolve axes for wall sliding.
    float distance = fmaxf(fabsf(d.x), fabsf(d.y));
    if (distance > 100000)
        return p;
    int steps = (int)ceilf(distance);
    if (steps < 1)
        return p;
    d.x /= steps;
    d.y /= steps;
    for (int i = 0; i < steps; i++) {
        Vector2 next = {p.x + d.x, p.y};
        if (!collisionFeet(m, next))
            p = next;
        next = (Vector2){p.x, p.y + d.y};
        if (!collisionFeet(m, next))
            p = next;
    }
    return p;
}
