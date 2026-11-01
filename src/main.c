#include "raylib.h"
#include "world.h"
#include "collision.h"
#include "scene_draw.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Integration based on 0aff630: preserve the group's five rooms, four-direction
// animation, automatic transitions, F1 mask overlay and coordinate inspection.
static Texture2D CarregarSprite(const char *path) {
    Image image = LoadImage(path);
    Texture2D texture = {0};
    if (image.data) {
        ImageAlphaCrop(&image, .05f); // align feet despite different transparent margins
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        if (texture.id)
            SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    }
    return texture;
}
static void LocalizarAssets(const char *arg0) {
    if (FileExists("assets/cenarios/salao_principal.png"))
        return;
    char path[4096];
    ssize_t count = readlink("/proc/self/exe", path, sizeof path - 1);
    if (count > 0)
        path[count] = '\0';
    else
        snprintf(path, sizeof path, "%s", arg0);
    char *slash = strrchr(path, '/');
    if (!slash)
        return;
    *slash = '\0';
    if (ChangeDirectory(path) && !FileExists("assets/cenarios/salao_principal.png"))
        ChangeDirectory("..");
}
static void CapturarTela(const char *path) {
    Image image = LoadImageFromScreen();
    if (image.data) {
        ExportImage(image, path);
        UnloadImage(image);
    }
}
static void DesenharDepuracao(int room, Vector2 john, Texture2D mask) {
    DrawTexture(mask, 0, 0, (Color){125, 255, 150, 210});
    DrawRectangleLinesEx(feet(john), 2, YELLOW);
    for (int i = 0; i < doorCount; i++) {
        const Door *d = &doors[i];
        if (d->from == room) {
            DrawRectangleLinesEx(d->area, 3, SKYBLUE);
            DrawText(TextFormat("P%d -> %s", i, d->label), d->area.x, d->area.y - 22, 20, SKYBLUE);
            if (d->sealA.x)
                DrawLineEx(d->sealA, d->sealB, 3, ORANGE);
        }
        if (d->to == room) {
            DrawCircleLines(d->arrival.x, d->arrival.y, 18, YELLOW);
            DrawText(TextFormat("Entrada %d", i), d->arrival.x + 22, d->arrival.y - 10, 19, YELLOW);
        }
    }
    DrawRectangle(20, 90, 440, 96, (Color){6, 8, 14, 230});
    DrawText("F1 - DEPURACAO", 34, 102, 22, YELLOW);
    DrawText(TextFormat("Pes: %.1f, %.1f | %.1f x %.1f", john.x, john.y, FOOT_W, FOOT_H), 34, 135, 19,
             RAYWHITE);
    DrawText("Azul: transicao | Amarelo: entrada", 34, 162, 18, SKYBLUE);
}
int main(int argc, char **argv) {
    int larguraInicial = 1280, alturaInicial = 720, limiteQuadros = 0;
    bool telaCheia = false;
    const char *captura = NULL, *galeria = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--fullscreen"))
            telaCheia = true;
        else if (!strcmp(argv[i], "--size") && i + 2 < argc) {
            larguraInicial = atoi(argv[++i]);
            alturaInicial = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--frames") && i + 1 < argc)
            limiteQuadros = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--capture") && i + 1 < argc)
            captura = argv[++i];
        else if (!strcmp(argv[i], "--gallery") && i + 1 < argc)
            galeria = argv[++i];
        else {
            fprintf(
                stderr,
                "Uso: %s [--fullscreen] [--size L A] [--frames N --capture arquivo.png] [--gallery pasta]\n",
                argv[0]);
            return 2;
        }
    }
    if (larguraInicial < 640 || alturaInicial < 360 || larguraInicial > 7680 || alturaInicial > 4320) {
        fprintf(stderr, "Resolucao invalida (minimo 640 x 360).\n");
        return 2;
    }
    LocalizarAssets(argv[0]);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(larguraInicial, alturaInicial, "VHS - A Ultima Sessao");
    if (!IsWindowReady())
        return 1;
    SetWindowMinSize(640, 360);
    if (telaCheia)
        ToggleBorderlessWindowed();
    SetTargetFPS(60);

    Texture2D cenarios[NUM_CENARIOS] = {0}, texturasMascaras[NUM_CENARIOS] = {0};
    CollisionMap mascaras[NUM_CENARIOS] = {0};
    Texture2D animacoes[NUM_DIRECOES][NUM_FRAMES] = {0};
    Texture2D johnParado[NUM_DIRECOES] = {0};
    const char *pastas[NUM_DIRECOES] = {"baixo", "cima", "direita", "esquerda"};
    bool erro = false;
    for (int i = 0; i < NUM_CENARIOS; i++) {
        cenarios[i] = LoadTexture(rooms[i].image);
        texturasMascaras[i] = LoadTexture(rooms[i].mask);
        if (!cenarios[i].id || cenarios[i].width != WORLD_W || cenarios[i].height != WORLD_H ||
            !texturasMascaras[i].id || !collisionLoadRoom(&mascaras[i], i)) {
            TraceLog(LOG_ERROR, "Cenario/mascara ausente ou invalido: %s", rooms[i].name);
            erro = true;
        } else {
            SetTextureFilter(cenarios[i], TEXTURE_FILTER_POINT);
            SetTextureFilter(texturasMascaras[i], TEXTURE_FILTER_POINT);
        }
    }
    for (int d = 0; d < NUM_DIRECOES; d++) {
        char caminho[256];
        snprintf(caminho, sizeof caminho, "assets/personagens/john/%s/john_parado_%s.png", pastas[d],
                 pastas[d]);
        johnParado[d] = CarregarSprite(caminho);
        if (!johnParado[d].id)
            erro = true;
        for (int f = 0; f < NUM_FRAMES; f++) {
            snprintf(caminho, sizeof caminho, "assets/personagens/john/%s/john_andando_%s%d.png", pastas[d],
                     pastas[d], f + 1);
            animacoes[d][f] = CarregarSprite(caminho);
            if (!animacoes[d][f].id)
                erro = true;
        }
    }
    for (int i = 0; i < NUM_CENARIOS && !erro; i++) {
        if (collisionFeet(&mascaras[i], rooms[i].entry)) {
            TraceLog(LOG_ERROR, "Entrada bloqueada: %s", rooms[i].name);
            erro = true;
        }
    }
    for (int i = 0; i < doorCount && !erro; i++) {
        if (collisionFeet(&mascaras[doors[i].to], doors[i].arrival)) {
            TraceLog(LOG_ERROR, "Destino da porta %d bloqueado", i);
            erro = true;
        }
    }
    RenderTexture2D quadro = {0};
    if (!erro) {
        quadro = LoadRenderTexture(WORLD_W, WORLD_H);
        if (!quadro.id)
            erro = true;
        else
            SetTextureFilter(quadro.texture, TEXTURE_FILTER_BILINEAR);
    }

    Vector2 john = rooms[SALAO].entry;
    int cenarioAtual = SALAO, direcao = BAIXO, frameAtual = 0;
    float tempoAnimacao = 0, duracaoFrame = .11f, esperaTransicao = 0, fade = 0;
    bool mostrarColisoes = false;
    int quadroAtual = 0, indiceGaleria = 0;
    while (!erro && !WindowShouldClose()) {
        float delta = fminf(GetFrameTime(), .25f);
        esperaTransicao = fmaxf(0, esperaTransicao - delta);
        fade = fmaxf(0, fade - delta);
        Vector2 movimento = {0};
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            movimento.x -= 1;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            movimento.x += 1;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            movimento.y -= 1;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            movimento.y += 1;
        bool andando = false, mudouCenario = false;
        if (movimento.x || movimento.y) {
            if (fabsf(movimento.y) > fabsf(movimento.x))
                direcao = movimento.y < 0 ? CIMA : BAIXO;
            else
                direcao = movimento.x < 0 ? ESQUERDA : DIREITA;
            float comprimento = hypotf(movimento.x, movimento.y);
            movimento.x /= comprimento;
            movimento.y /= comprimento;
            Vector2 anterior = john;
            john =
                collisionMove(&mascaras[cenarioAtual], john,
                              (Vector2){movimento.x * JOHN_SPEED * delta, movimento.y * JOHN_SPEED * delta});
            andando = hypotf(john.x - anterior.x, john.y - anterior.y) > .01f;
            mudouCenario = worldTransition(&cenarioAtual, &john, &direcao, movimento, &esperaTransicao);
            if (mudouCenario) {
                fade = .14f;
                andando = false;
                TraceLog(LOG_INFO, "Passagem: %s | entrada %.0f, %.0f", rooms[cenarioAtual].name, john.x,
                         john.y);
            }
        }
        if (andando) {
            tempoAnimacao += delta;
            while (tempoAnimacao >= duracaoFrame) {
                tempoAnimacao -= duracaoFrame;
                frameAtual = (frameAtual + 1) % NUM_FRAMES;
            }
        } else {
            frameAtual = 0;
            tempoAnimacao = 0;
        }
        if (IsKeyPressed(KEY_F1))
            mostrarColisoes = !mostrarColisoes;
        if (IsKeyPressed(KEY_F11))
            ToggleBorderlessWindowed();
        if (mostrarColisoes && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 p = screenToWorld(GetMousePosition(), viewport(GetScreenWidth(), GetScreenHeight()));
            printf("Ponto: X = %.2f%%, Y = %.2f%% | Imagem: %.0f, %.0f\n", 100 * p.x / WORLD_W,
                   100 * p.y / WORLD_H, p.x, p.y);
            fflush(stdout);
        }
        if (galeria) {
            cenarioAtual = indiceGaleria / 2;
            john = rooms[cenarioAtual].entry;
            mostrarColisoes = indiceGaleria % 2;
            direcao = BAIXO;
            andando = false;
            fade = 0;
        }
        Texture2D atual = andando && !mudouCenario ? animacoes[direcao][frameAtual] : johnParado[direcao];
        BeginTextureMode(quadro);
        ClearBackground(BLACK);
        DrawTexture(cenarios[cenarioAtual], 0, 0, WHITE);
        sceneDrawJohn(cenarios[cenarioAtual], atual, cenarioAtual, john);
        if (fade > 0)
            DrawRectangle(0, 0, WORLD_W, WORLD_H, Fade(BLACK, fade * 2));
        if (mostrarColisoes)
            DesenharDepuracao(cenarioAtual, john, texturasMascaras[cenarioAtual]);
        DrawRectangle(18, 16, 270, 34, (Color){6, 8, 14, 195});
        DrawText(rooms[cenarioAtual].name, 30, 24, 20, RAYWHITE);
        DrawRectangle(0, WORLD_H - 29, WORLD_W, 29, (Color){6, 8, 14, 195});
        DrawText("WASD / SETAS: mover e atravessar portas     F1: colisoes     F11: tela cheia     ESC: sair",
                 24, WORLD_H - 22, 17, LIGHTGRAY);
        EndTextureMode();
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(quadro.texture, (Rectangle){0, 0, WORLD_W, -WORLD_H},
                       viewport(GetScreenWidth(), GetScreenHeight()), (Vector2){0, 0}, 0, WHITE);
        EndDrawing();
        if (galeria && quadroAtual % 3 == 2) {
            char path[4096];
            snprintf(path, sizeof path, "%s/%d-%s.png", galeria, indiceGaleria / 2,
                     mostrarColisoes ? "debug" : "normal");
            CapturarTela(path);
            if (++indiceGaleria == 10)
                break;
        }
        quadroAtual++;
        if (limiteQuadros > 0 && quadroAtual >= limiteQuadros) {
            if (captura)
                CapturarTela(captura);
            break;
        }
    }
    if (quadro.id)
        UnloadRenderTexture(quadro);
    for (int i = 0; i < NUM_CENARIOS; i++) {
        if (cenarios[i].id)
            UnloadTexture(cenarios[i]);
        if (texturasMascaras[i].id)
            UnloadTexture(texturasMascaras[i]);
        collisionFree(&mascaras[i]);
    }
    for (int d = 0; d < NUM_DIRECOES; d++) {
        if (johnParado[d].id)
            UnloadTexture(johnParado[d]);
        for (int f = 0; f < NUM_FRAMES; f++)
            if (animacoes[d][f].id)
                UnloadTexture(animacoes[d][f]);
    }
    CloseWindow();
    return erro ? 1 : 0;
}
