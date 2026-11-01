#ifndef TEST_NAVIGATION_H
#define TEST_NAVIGATION_H
#include "collision.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define NAV_STEP 8
#define NAV_W (WORLD_W / NAV_STEP)
#define NAV_H (WORLD_H / NAV_STEP)
#define NAV_N (NAV_W * NAV_H)
typedef struct {
    unsigned char valid[NAV_N];
    int prev[NAV_N], queue[NAV_N];
} Nav;
static Vector2 navPoint(int n) { return (Vector2){(n % NAV_W) * NAV_STEP + 4, (n / NAV_W) * NAV_STEP + 4}; }
static int navIndex(Vector2 p) {
    int x = (int)p.x / NAV_STEP, y = (int)p.y / NAV_STEP;
    return x < 0 || y < 0 || x >= NAV_W || y >= NAV_H ? -1 : y * NAV_W + x;
}
static void navBuild(Nav *n, const CollisionMap *m) {
    for (int i = 0; i < NAV_N; i++)
        n->valid[i] = !collisionFeet(m, navPoint(i));
}
static bool navEdge(const CollisionMap *m, Vector2 a, Vector2 b) {
    Vector2 end = collisionMove(m, a, (Vector2){b.x - a.x, b.y - a.y});
    return hypotf(end.x - b.x, end.y - b.y) < .01f;
}
static int navSearch(Nav *n, const CollisionMap *m, Vector2 start) {
    memset(n->prev, 255, sizeof n->prev);
    int s = navIndex(start);
    if (s < 0 || !n->valid[s] || !navEdge(m, start, navPoint(s)))
        return 0;
    n->prev[s] = s;
    n->queue[0] = s;
    int head = 0, tail = 1;
    const int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
    while (head < tail) {
        int a = n->queue[head++], x = a % NAV_W, y = a / NAV_W;
        for (int d = 0; d < 4; d++) {
            int xx = x + dx[d], yy = y + dy[d];
            if (xx < 0 || xx >= NAV_W || yy < 0 || yy >= NAV_H)
                continue;
            int b = yy * NAV_W + xx;
            if (n->valid[b] && n->prev[b] < 0 && navEdge(m, navPoint(a), navPoint(b))) {
                n->prev[b] = a;
                n->queue[tail++] = b;
            }
        }
    }
    return tail;
}
static bool navReach(const Nav *n, const CollisionMap *m, Vector2 p) {
    if (collisionFeet(m, p))
        return false;
    int x = (int)p.x / NAV_STEP, y = (int)p.y / NAV_STEP;
    for (int dy = -2; dy <= 2; dy++)
        for (int dx = -2; dx <= 2; dx++) {
            int xx = x + dx, yy = y + dy;
            if (xx < 0 || xx >= NAV_W || yy < 0 || yy >= NAV_H)
                continue;
            int i = yy * NAV_W + xx;
            if (n->prev[i] >= 0 && navEdge(m, navPoint(i), p))
                return true;
        }
    return false;
}
#endif
