#ifndef VHS_SCENE_DRAW_H
#define VHS_SCENE_DRAW_H
#include "world.h"
// Only repaint foreground furniture when John is behind its floor footprint.
void sceneDrawJohn(Texture2D background, Texture2D john, int room, Vector2 feetPosition);
#endif
