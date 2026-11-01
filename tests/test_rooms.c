#include "navigation.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
static int checks = 0, failures = 0;
#define CHECK(test)                                                                                          \
    do {                                                                                                     \
        checks++;                                                                                            \
        if (!(test)) {                                                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #test);                                  \
            failures++;                                                                                      \
        }                                                                                                    \
    } while (0)
static CollisionMap maps[NUM_CENARIOS];
static void testNavigation(void) {
    Nav *nav = malloc(sizeof *nav);
    CHECK(nav != NULL);
    if (!nav)
        return;
    for (int room = 0; room < 5; room++) {
        CHECK(!collisionFeet(&maps[room], rooms[room].entry));
        navBuild(nav, &maps[room]);
        int count = navSearch(nav, &maps[room], rooms[room].entry);
        printf("NAV %s: %d células alcançáveis\n", rooms[room].name, count);
        CHECK(count > 100);
        // A reachable cell near any image edge would reveal a hole in the perimeter.
        for (int a = 0; a < NAV_N; a++)
            if (nav->prev[a] >= 0) {
                Vector2 p = navPoint(a);
                CHECK(p.x > 20 && p.y > 20 && p.x < WORLD_W - 4 && p.y < WORLD_H - 4);
            }
        for (int i = 0; i < doorCount; i++) {
            if (doors[i].to == room) {
                bool ok = navReach(nav, &maps[room], doors[i].arrival);
                if (!ok)
                    fprintf(stderr, "Entrada inacessível: porta %d\n", i);
                CHECK(ok);
            }
            if (doors[i].from == room) {
                Vector2 p = {doors[i].area.x + doors[i].area.width / 2,
                             doors[i].area.y + doors[i].area.height / 2};
                bool ok = navReach(nav, &maps[room], p);
                if (!ok)
                    fprintf(stderr, "Porta inacessível: %d (%g,%g)\n", i, p.x, p.y);
                CHECK(ok);
            }
        }
        // Sweep into every accessible grid boundary from 8 directions, with a
        // displacement far larger than the obstacle. It must stop at the FIRST edge.
        int edgeChecks = 0;
        for (int a = 0; a < NAV_N; a++)
            if (nav->prev[a] >= 0) {
                Vector2 start = navPoint(a);
                for (int dx = -1; dx <= 1; dx++)
                    for (int dy = -1; dy <= 1; dy++) {
                        if (!dx && !dy)
                            continue;
                        Vector2 near = {start.x + dx * NAV_STEP, start.y + dy * NAV_STEP};
                        if (!collisionFeet(&maps[room], near))
                            continue;
                        // Axis sweeps have an unambiguous first blocking boundary; diagonals may
                        // slide legitimately, so validate their entire final box separately.
                        Vector2 hit = collisionMove(&maps[room], start, (Vector2){dx * 512, dy * 512});
                        CHECK(!collisionFeet(&maps[room], hit));
                        if (!dx || !dy)
                            CHECK(hypotf(hit.x - start.x, hit.y - start.y) <= NAV_STEP + 1);
                        edgeChecks++;
                    }
            }
        printf("WALL %s: %d abordagens de paredes/cantos\n", rooms[room].name, edgeChecks);
        // Deterministic walks with long/short frames and rapid diagonal direction changes.
        unsigned rng = 101 + room;
        Vector2 p = rooms[room].entry;
        for (int i = 0; i < 2500; i++) {
            rng = rng * 1664525u + 1013904223u;
            float x = (float)((rng >> 16) % 3) - 1;
            rng = rng * 1664525u + 1013904223u;
            float y = (float)((rng >> 16) % 3) - 1;
            float times[] = {1.f / 240, 1.f / 60, .1f, .25f, 1.5f};
            float dt = times[i % 5];
            Vector2 next = collisionMove(&maps[room], p, (Vector2){x * JOHN_SPEED * dt, y * JOHN_SPEED * dt});
            CHECK(!collisionFeet(&maps[room], next));
            p = next;
        }
    }
    free(nav);
}
static void testCoordinates(void) {
    const int sizes[][2] = {{1280, 720}, {1672, 941}, {1920, 1080}, {1024, 768},
                            {800, 600},  {640, 360},  {2560, 1080}};
    for (int s = 0; s < 7; s++) {
        Rectangle v = viewport(sizes[s][0], sizes[s][1]);
        CHECK(fabsf(v.width / WORLD_W - v.height / WORLD_H) < .00001f);
        for (int i = 0; i < 40; i++) {
            Vector2 p = {i * 40.1f, i * 22.2f};
            Vector2 q = screenToWorld(worldToScreen(p, v), v);
            CHECK(hypotf(p.x - q.x, p.y - q.y) < .001f);
        }
    }
}
static void testCollisionPixels(void) {
    // One-pixel line and an interior pixel prove whole-box coverage and no tunneling.
    CollisionMap m = {0};
    m.room = -1;
    m.width = WORLD_W;
    m.height = WORLD_H;
    m.ready = true;
    m.sum = calloc((WORLD_W + 1) * (WORLD_H + 1), sizeof *m.sum);
    CHECK(m.sum != NULL);
    if (!m.sum)
        return;
    for (int y = 0; y < WORLD_H; y++) {
        unsigned row = 0;
        for (int x = 0; x < WORLD_W; x++) {
            row += (x == 500) || (x == 800 && y == 499);
            m.sum[(y + 1) * (WORLD_W + 1) + x + 1] = row + m.sum[y * (WORLD_W + 1) + x + 1];
        }
    }
    CHECK(collisionFeet(&m, (Vector2){800, 505}));
    CHECK(!collisionFeet(&m, (Vector2){400, 500}));
    Vector2 end = collisionMove(&m, (Vector2){400, 500}, (Vector2){1200, 300});
    CHECK(end.x < 500 - FOOT_W / 2);
    CHECK(fabsf(end.y - 800) < .01f);
    CHECK(collisionFeet(&m, (Vector2){1, 1}));
    CHECK(collisionFeet(&m, (Vector2){WORLD_W, 500}));
    CHECK(collisionRect(&m, (Rectangle){NAN, 0, 30, 10}));
    CHECK(collisionRect(&m, (Rectangle){0, 0, INFINITY, 10}));
    CHECK(collisionRect(&m, (Rectangle){1e30f, 0, 30, 10}));
    collisionFree(&m);
}
// Follow actual collision-resolved movements along the navigation mesh; no
// teleport to a trigger. The only discontinuity must be worldTransition itself.
static bool moveToward(int *room, Vector2 *p, int *facing, float *cooldown, Vector2 target, float frameTime) {
    for (int frame = 0; frame < 1000; frame++) {
        Vector2 delta = {target.x - p->x, target.y - p->y};
        float length = hypotf(delta.x, delta.y);
        if (length < .01f)
            return false;
        float dt = fminf(frameTime, length / JOHN_SPEED);
        Vector2 movement = {delta.x / length, delta.y / length}, before = *p;
        *cooldown = fmaxf(0, *cooldown - dt);
        *p = collisionMove(&maps[*room], *p,
                           (Vector2){movement.x * JOHN_SPEED * dt, movement.y * JOHN_SPEED * dt});
        CHECK(!collisionFeet(&maps[*room], *p));
        if (worldTransition(room, p, facing, movement, cooldown))
            return true;
        if (hypotf(p->x - before.x, p->y - before.y) < .001f) {
            CHECK(false);
            return false;
        }
    }
    CHECK(false);
    return false;
}
static void testRoutes(void) {
    Nav *nav = malloc(sizeof *nav);
    CHECK(nav != NULL);
    if (!nav)
        return;
    // Eight directed connections, visited from the actual previous arrival.
    const int route[] = {0, 1, 2, 3, 4, 6, 7, 5};
    const float rates[] = {1.f / 144, 1.f / 60, 1.f / 30, .25f};
    for (unsigned rate = 0; rate < sizeof rates / sizeof *rates; rate++) {
        int room = SALAO, facing = BAIXO;
        Vector2 p = rooms[SALAO].entry;
        float cooldown = 0;
        for (int lap = 0; lap < 3; lap++)
            for (unsigned step = 0; step < sizeof route / sizeof *route; step++) {
                const Door *door = &doors[route[step]];
                CHECK(room == door->from);
                navBuild(nav, &maps[room]);
                CHECK(navSearch(nav, &maps[room], p) > 0);
                Vector2 target = {door->area.x + door->area.width / 2, door->area.y + door->area.height / 2};
                int node = navIndex(target), path[NAV_N], n = 0;
                CHECK(node >= 0 && nav->prev[node] >= 0);
                if (node < 0 || nav->prev[node] < 0)
                    continue;
                while (nav->prev[node] != node && n < NAV_N) {
                    path[n++] = node;
                    node = nav->prev[node];
                }
                CHECK(n < NAV_N);
                path[n++] = node;
                bool crossed = false;
                for (int i = n - 1; i >= 0 && !crossed; i--)
                    crossed = moveToward(&room, &p, &facing, &cooldown, navPoint(path[i]), rates[rate]);
                if (!crossed)
                    crossed = moveToward(&room, &p, &facing, &cooldown, target, rates[rate]);
                if (!crossed) {
                    cooldown = 0;
                    crossed = worldTransition(&room, &p, &facing, door->direction, &cooldown);
                }
                CHECK(crossed);
                CHECK(room == door->to);
                CHECK(facing == door->facing);
                CHECK(hypotf(p.x - door->arrival.x, p.y - door->arrival.y) < .01f);
                CHECK(!collisionFeet(&maps[room], p));
                // Neither waiting, continuing, nor changing direction bounces back.
                CHECK(!worldTransition(&room, &p, &facing, (Vector2){0, 0}, &cooldown));
                for (int j = 0; j < doorCount; j++)
                    if (doors[j].from == room) {
                        CHECK(!CheckCollisionPointRec(p, doors[j].area));
                        float zero = 0;
                        CHECK(!worldTransition(&room, &p, &facing, doors[j].direction, &zero));
                    }
            }
        printf("ROTAS: 3 voltas completas, 24 passagens, passo maximo %.4f s\n", rates[rate]);
    }
    free(nav);
}
static void testPortalDirection(void) {
    const float times[] = {1.f / 144, 1.f / 60, 1.f / 30, .1f, .25f};
    for (int i = 0; i < doorCount; i++) {
        const Door *d = &doors[i];
        Vector2 p = {d->area.x + d->area.width / 2, d->area.y + d->area.height / 2};
        CHECK(worldDoorAt(d->from, p, d->direction) == i);
        CHECK(worldDoorAt(d->from, p, (Vector2){-d->direction.x, -d->direction.y}) == -1);
        CHECK(worldDoorAt(d->from, p, (Vector2){0, 0}) == -1);
        int room = d->from, facing = 0;
        float cooldown = .2f;
        CHECK(!worldTransition(&room, &p, &facing, d->direction, &cooldown));
        CHECK(room == d->from);
        // Approach the trigger in full-size frames, including a 250 ms stall.
        for (unsigned t = 0; t < sizeof times / sizeof *times; t++) {
            Vector2 start = {d->area.x + d->area.width / 2, d->area.y + d->area.height / 2};
            if (d->direction.x < 0)
                start.x = d->area.x + d->area.width + 24;
            if (d->direction.x > 0)
                start.x = d->area.x - 24;
            if (d->direction.y < 0)
                start.y = d->area.y + d->area.height + 24;
            if (d->direction.y > 0)
                start.y = d->area.y - 24;
            room = d->from;
            cooldown = 0;
            CHECK(!collisionFeet(&maps[room], start));
            for (int frame = 0; frame < 60 && room == d->from; frame++) {
                start = collisionMove(&maps[room], start,
                                      (Vector2){d->direction.x * JOHN_SPEED * times[t],
                                                d->direction.y * JOHN_SPEED * times[t]});
                worldTransition(&room, &start, &facing, d->direction, &cooldown);
            }
            CHECK(room == d->to);
            CHECK(!collisionFeet(&maps[room], start));
        }
    }
}
int main(void) {
    SetTraceLogLevel(LOG_WARNING);
    for (int i = 0; i < NUM_CENARIOS; i++) {
        bool ok = collisionLoadRoom(&maps[i], i);
        CHECK(ok);
        if (!ok)
            return 1;
    }
    testCoordinates();
    testCollisionPixels();
    testNavigation();
    testPortalDirection();
    testRoutes();
    for (int i = 0; i < NUM_CENARIOS; i++)
        collisionFree(&maps[i]);
    printf("RESULTADO: %d verificacoes, %d falhas\n", checks, failures);
    return failures ? 1 : 0;
}
