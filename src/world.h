#ifndef VHS_WORLD_H
#define VHS_WORLD_H
#include "raylib.h"

#define WORLD_W 1672
#define WORLD_H 941
#define NUM_CENARIOS 5
#define NUM_DIRECOES 4
#define NUM_FRAMES 4
#define JOHN_HEIGHT (WORLD_H * 0.15f)
#define JOHN_SPEED (WORLD_H * 0.35f)
#define FOOT_W (JOHN_HEIGHT * 0.24f)
#define FOOT_H (JOHN_HEIGHT * 0.12f)

enum Direcao { BAIXO = 0, CIMA, DIREITA, ESQUERDA };
enum Cenario { SALAO = 0, BANHEIRO, ESCRITORIO, ESTOQUE, SALA_PROJECAO };

typedef struct {
    const char *name, *image, *mask;
    Vector2 entry;
} Room;

typedef struct {
    int from, to;
    Rectangle area;
    Vector2 arrival;
    int facing;
    const char *label;
    Vector2 sealA, sealB; // portal threshold; the PNG retains its doorway opening
    Vector2 direction;
} Door;

extern const Room rooms[NUM_CENARIOS];
extern const Door doors[];
extern const int doorCount;
Rectangle feet(Vector2 position);
Rectangle viewport(int width, int height);
Vector2 screenToWorld(Vector2 point, Rectangle view);
Vector2 worldToScreen(Vector2 point, Rectangle view);
int worldDoorAt(int room, Vector2 position, Vector2 movement);
bool worldTransition(int *room, Vector2 *position, int *facing, Vector2 movement, float *cooldown);
#endif
