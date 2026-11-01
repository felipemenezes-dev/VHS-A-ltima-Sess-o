#include "scene_draw.h"
void sceneDrawJohn(Texture2D background, Texture2D john, int room, Vector2 position) {
    typedef struct {
        int room;
        Rectangle rect;
        float depth;
    } Foreground;
    static const Foreground fronts[] = {
        {SALAO, {323, 245, 94, 269}, 514},          {SALAO, {574, 245, 89, 274}, 519},
        {SALAO, {955, 245, 92, 274}, 519},          {SALAO, {1256, 245, 96, 284}, 529},
        {SALAO, {1073, 520, 127, 158}, 678},        {ESCRITORIO, {665, 228, 442, 176}, 404},
        {ESCRITORIO, {524, 290, 102, 132}, 422},    {ESTOQUE, {518, 279, 84, 368}, 647},
        {ESTOQUE, {617, 276, 88, 339}, 615},        {ESTOQUE, {1248, 549, 171, 151}, 700},
        {SALA_PROJECAO, {680, 250, 311, 162}, 412}, {SALA_PROJECAO, {1125, 420, 152, 153}, 573}};
    float width = JOHN_HEIGHT * john.width / john.height;
    DrawTexturePro(john, (Rectangle){0, 0, john.width, john.height},
                   (Rectangle){position.x - width / 2, position.y - JOHN_HEIGHT, width, JOHN_HEIGHT},
                   (Vector2){0, 0}, 0, WHITE);
    for (unsigned i = 0; i < sizeof fronts / sizeof *fronts; i++) {
        const Foreground *f = &fronts[i];
        if (f->room != room || position.y >= f->depth || position.x + 40 < f->rect.x ||
            position.x - 40 > f->rect.x + f->rect.width)
            continue;
        DrawTextureRec(background, f->rect, (Vector2){f->rect.x, f->rect.y}, WHITE);
    }
}
