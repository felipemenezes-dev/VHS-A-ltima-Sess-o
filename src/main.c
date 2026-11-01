#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define NUM_DIRECOES 4
#define NUM_FRAMES 4
#define NUM_CENARIOS 5

enum Direcao {
    BAIXO = 0,
    CIMA,
    DIREITA,
    ESQUERDA
};

enum Cenario {
    SALAO = 0,
    BANHEIRO,
    ESCRITORIO,
    ESTOQUE,
    SALA_PROJECAO
};

static Rectangle CaixaColisaoJohn(Vector2 john, float alturaJohn) {
    float larguraCaixa = alturaJohn * 0.24f;
    float alturaCaixa = alturaJohn * 0.12f;

    return (Rectangle) {
        john.x - larguraCaixa / 2.0f,
        john.y - alturaCaixa,
        larguraCaixa,
        alturaCaixa
    };
}

static bool ColideComMascara(
    Image mascara,
    Rectangle caixa,
    float larguraTela,
    float alturaTela
) {
    if (mascara.data == NULL ||
        larguraTela <= 0.0f ||
        alturaTela <= 0.0f) {
        return false;
    }

    float escalaX = (float)mascara.width / larguraTela;
    float escalaY = (float)mascara.height / alturaTela;

    int xInicial = (int)floorf(caixa.x * escalaX);
    int yInicial = (int)floorf(caixa.y * escalaY);
    int xFinal = (int)ceilf((caixa.x + caixa.width) * escalaX);
    int yFinal = (int)ceilf((caixa.y + caixa.height) * escalaY);

    if (xInicial < 0 ||
        yInicial < 0 ||
        xFinal >= mascara.width ||
        yFinal >= mascara.height) {
        return true;
    }

    for (int y = yInicial; y <= yFinal; y++) {
        for (int x = xInicial; x <= xFinal; x++) {
            Color pixel = GetImageColor(mascara, x, y);

            if (pixel.a > 0) {
                return true;
            }
        }
    }

    return false;
}

int main(void) {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);

    int monitor = GetCurrentMonitor();

    InitWindow(
        GetMonitorWidth(monitor),
        GetMonitorHeight(monitor),
        "VHS - A Ultima Sessao"
    );

    SetTargetFPS(60);

    const char *caminhosCenarios[NUM_CENARIOS] = {
        "assets/cenarios/salao_principal.png",
        "assets/cenarios/banheiro.png",
        "assets/cenarios/escritorio.png",
        "assets/cenarios/estoque.png",
        "assets/cenarios/sala_projecao.png"
    };

    Texture2D cenarios[NUM_CENARIOS] = {0};

    const char *pastas[NUM_DIRECOES] = {
        "baixo",
        "cima",
        "direita",
        "esquerda"
    };

    const char *nomes[NUM_DIRECOES] = {
        "baixo",
        "cima",
        "direita",
        "esquerda"
    };

    Texture2D animacoes[NUM_DIRECOES][NUM_FRAMES] = {0};
    Texture2D johnParado[NUM_DIRECOES] = {0};

    Image mascaraSalao = {0};
    Image mascaraBanheiro = {0};

    Texture2D texturaMascaraSalao = {0};
    Texture2D texturaMascaraBanheiro = {0};

    bool erro = false;

    for (int i = 0; i < NUM_CENARIOS; i++) {
        cenarios[i] = LoadTexture(caminhosCenarios[i]);

        if (cenarios[i].id == 0) {
            TraceLog(
                LOG_ERROR,
                "Nao foi possivel carregar: %s",
                caminhosCenarios[i]
            );

            erro = true;
        } else {
            SetTextureFilter(cenarios[i], TEXTURE_FILTER_POINT);
        }
    }

    mascaraSalao = LoadImage("assets/cenarios/colisoes_salao.png");

    if (mascaraSalao.data == NULL) {
        TraceLog(
            LOG_ERROR,
            "Nao foi possivel carregar: assets/cenarios/colisoes_salao.png"
        );

        erro = true;
    } else {
        texturaMascaraSalao = LoadTextureFromImage(mascaraSalao);

        if (texturaMascaraSalao.id == 0) {
            TraceLog(
                LOG_ERROR,
                "Nao foi possivel criar a visualizacao da mascara do salao"
            );

            erro = true;
        } else {
            SetTextureFilter(texturaMascaraSalao, TEXTURE_FILTER_POINT);
        }
    }

    mascaraBanheiro = LoadImage("assets/cenarios/colisoes_banheiro.png");

    if (mascaraBanheiro.data == NULL) {
        TraceLog(
            LOG_ERROR,
            "Nao foi possivel carregar: assets/cenarios/colisoes_banheiro.png"
        );

        erro = true;
    } else {
        texturaMascaraBanheiro = LoadTextureFromImage(mascaraBanheiro);

        if (texturaMascaraBanheiro.id == 0) {
            TraceLog(
                LOG_ERROR,
                "Nao foi possivel criar a visualizacao da mascara do banheiro"
            );

            erro = true;
        } else {
            SetTextureFilter(texturaMascaraBanheiro, TEXTURE_FILTER_POINT);
        }
    }

    for (int d = 0; d < NUM_DIRECOES; d++) {
        char caminho[256];

        snprintf(
            caminho,
            sizeof(caminho),
            "assets/personagens/john/%s/john_parado_%s.png",
            pastas[d],
            nomes[d]
        );

        johnParado[d] = LoadTexture(caminho);

        if (johnParado[d].id == 0) {
            TraceLog(LOG_ERROR, "Nao foi possivel carregar: %s", caminho);
            erro = true;
        } else {
            SetTextureFilter(johnParado[d], TEXTURE_FILTER_POINT);
        }

        for (int f = 0; f < NUM_FRAMES; f++) {
            snprintf(
                caminho,
                sizeof(caminho),
                "assets/personagens/john/%s/john_andando_%s%d.png",
                pastas[d],
                nomes[d],
                f + 1
            );

            animacoes[d][f] = LoadTexture(caminho);

            if (animacoes[d][f].id == 0) {
                TraceLog(LOG_ERROR, "Nao foi possivel carregar: %s", caminho);
                erro = true;
            } else {
                SetTextureFilter(animacoes[d][f], TEXTURE_FILTER_POINT);
            }
        }
    }

    if (erro) {
        for (int i = 0; i < NUM_CENARIOS; i++) {
            if (cenarios[i].id != 0) {
                UnloadTexture(cenarios[i]);
            }
        }

        for (int d = 0; d < NUM_DIRECOES; d++) {
            if (johnParado[d].id != 0) {
                UnloadTexture(johnParado[d]);
            }

            for (int f = 0; f < NUM_FRAMES; f++) {
                if (animacoes[d][f].id != 0) {
                    UnloadTexture(animacoes[d][f]);
                }
            }
        }

        if (texturaMascaraSalao.id != 0) {
            UnloadTexture(texturaMascaraSalao);
        }

        if (texturaMascaraBanheiro.id != 0) {
            UnloadTexture(texturaMascaraBanheiro);
        }

        if (mascaraSalao.data != NULL) {
            UnloadImage(mascaraSalao);
        }

        if (mascaraBanheiro.data != NULL) {
            UnloadImage(mascaraBanheiro);
        }

        CloseWindow();
        return 1;
    }

    float larguraTela = (float)GetScreenWidth();
    float alturaTela = (float)GetScreenHeight();

    Vector2 john = {
        larguraTela * 0.50f,
        alturaTela * 0.62f
    };

    float velocidade = alturaTela * 0.35f;

    int cenarioAtual = SALAO;
    int direcao = BAIXO;
    int frameAtual = 0;

    float tempoAnimacao = 0.0f;
    float duracaoFrame = 0.11f;

    bool mostrarColisoes = false;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        larguraTela = (float)GetScreenWidth();
        alturaTela = (float)GetScreenHeight();

        float alturaJohn = alturaTela * 0.15f;
        Vector2 mouse = GetMousePosition();

        float movimentoX = 0.0f;
        float movimentoY = 0.0f;

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            movimentoX -= 1.0f;
        }

        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            movimentoX += 1.0f;
        }

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            movimentoY -= 1.0f;
        }

        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
            movimentoY += 1.0f;
        }

        bool andando = movimentoX != 0.0f || movimentoY != 0.0f;

        if (andando) {
            if (fabsf(movimentoY) > fabsf(movimentoX)) {
                direcao = movimentoY < 0.0f ? CIMA : BAIXO;
            } else {
                direcao = movimentoX > 0.0f ? DIREITA : ESQUERDA;
            }

            float comprimento = sqrtf(
                movimentoX * movimentoX +
                movimentoY * movimentoY
            );

            movimentoX /= comprimento;
            movimentoY /= comprimento;

            float deslocamentoX = movimentoX * velocidade * delta;
            float deslocamentoY = movimentoY * velocidade * delta;

            if (cenarioAtual == SALAO || cenarioAtual == BANHEIRO) {
                Image mascaraAtual = (cenarioAtual == SALAO)
                    ? mascaraSalao
                    : mascaraBanheiro;

                john.x += deslocamentoX;

                if (ColideComMascara(
                    mascaraAtual,
                    CaixaColisaoJohn(john, alturaJohn),
                    larguraTela,
                    alturaTela
                )) {
                    john.x -= deslocamentoX;
                }

                john.y += deslocamentoY;

                if (ColideComMascara(
                    mascaraAtual,
                    CaixaColisaoJohn(john, alturaJohn),
                    larguraTela,
                    alturaTela
                )) {
                    john.y -= deslocamentoY;
                }
            } else {
                john.x += deslocamentoX;
                john.y += deslocamentoY;
            }

            tempoAnimacao += delta;

            while (tempoAnimacao >= duracaoFrame) {
                tempoAnimacao -= duracaoFrame;
                frameAtual = (frameAtual + 1) % NUM_FRAMES;
            }
        } else {
            frameAtual = 0;
            tempoAnimacao = 0.0f;
        }

        if (IsKeyPressed(KEY_F1)) {
            mostrarColisoes = !mostrarColisoes;
        }

        if (
            mostrarColisoes &&
            (cenarioAtual == SALAO || cenarioAtual == BANHEIRO) &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        ) {
            printf(
                "Ponto: X = %.2f%%, Y = %.2f%% | Pixel: %.0f, %.0f\n",
                mouse.x / larguraTela * 100.0f,
                mouse.y / alturaTela * 100.0f,
                mouse.x,
                mouse.y
            );

            fflush(stdout);
        }

        bool mudouCenario = false;

        if (cenarioAtual == SALAO) {
            Rectangle portaBanheiro = {
                0.0f,
                alturaTela * 0.35f,
                larguraTela * 0.055f,
                alturaTela * 0.29f
            };

            Rectangle portaEscritorio = {
                larguraTela * 0.255f,
                alturaTela * 0.045f,
                larguraTela * 0.095f,
                alturaTela * 0.16f
            };

            Rectangle portaEstoque = {
                larguraTela * 0.655f,
                alturaTela * 0.045f,
                larguraTela * 0.095f,
                alturaTela * 0.16f
            };

            if (
                movimentoX < 0.0f &&
                CheckCollisionPointRec(john, portaBanheiro)
            ) {
                cenarioAtual = BANHEIRO;

                john.x = larguraTela * 0.855f;
                john.y = alturaTela * 0.68f;

                direcao = ESQUERDA;
                frameAtual = 0;
                tempoAnimacao = 0.0f;

                mudouCenario = true;
            } else if (CheckCollisionPointRec(john, portaEscritorio)) {
                cenarioAtual = ESCRITORIO;

                john.x = larguraTela * 0.50f;
                john.y = alturaTela * 0.73f;

                direcao = CIMA;
                frameAtual = 0;
                tempoAnimacao = 0.0f;

                mudouCenario = true;
            } else if (CheckCollisionPointRec(john, portaEstoque)) {
                cenarioAtual = ESTOQUE;

                john.x = larguraTela * 0.50f;
                john.y = alturaTela * 0.73f;

                direcao = CIMA;
                frameAtual = 0;
                tempoAnimacao = 0.0f;

                mudouCenario = true;
            }
        } else if (cenarioAtual == BANHEIRO) {
            Rectangle saidaBanheiro = {
                larguraTela * 0.89f,
                alturaTela * 0.50f,
                larguraTela * 0.11f,
                alturaTela * 0.25f
            };

            if (
                movimentoX > 0.0f &&
                CheckCollisionPointRec(john, saidaBanheiro)
            ) {
                cenarioAtual = SALAO;

                john.x = larguraTela * 0.06f;
                john.y = alturaTela * 0.53f;

                direcao = DIREITA;
                frameAtual = 0;
                tempoAnimacao = 0.0f;

                mudouCenario = true;
            }
        } else {
            if (cenarioAtual == ESTOQUE) {
                Rectangle portaProjecao = {
                    larguraTela * 0.44f,
                    alturaTela * 0.03f,
                    larguraTela * 0.12f,
                    alturaTela * 0.21f
                };

                if (
                    movimentoY < 0.0f &&
                    CheckCollisionPointRec(john, portaProjecao)
                ) {
                    cenarioAtual = SALA_PROJECAO;

                    john.x = larguraTela * 0.50f;
                    john.y = alturaTela * 0.73f;

                    direcao = CIMA;
                    frameAtual = 0;
                    tempoAnimacao = 0.0f;

                    mudouCenario = true;
                }
            }

            Rectangle saidaComodo = {
                larguraTela * 0.38f,
                alturaTela * 0.86f,
                larguraTela * 0.24f,
                alturaTela * 0.13f
            };

            if (CheckCollisionPointRec(john, saidaComodo)) {
                int comodoAnterior = cenarioAtual;

                if (comodoAnterior == SALA_PROJECAO) {
                    cenarioAtual = ESTOQUE;

                    john.x = larguraTela * 0.50f;
                    john.y = alturaTela * 0.23f;
                } else {
                    cenarioAtual = SALAO;

                    if (comodoAnterior == ESCRITORIO) {
                        john.x = larguraTela * 0.30f;
                        john.y = alturaTela * 0.23f;
                    } else if (comodoAnterior == ESTOQUE) {
                        john.x = larguraTela * 0.70f;
                        john.y = alturaTela * 0.23f;
                    }
                }

                direcao = BAIXO;
                frameAtual = 0;
                tempoAnimacao = 0.0f;

                mudouCenario = true;
            }
        }

        Texture2D atual;

        if (andando && !mudouCenario) {
            atual = animacoes[direcao][frameAtual];
        } else {
            atual = johnParado[direcao];
        }

        float proporcao = (float)atual.width / (float)atual.height;
        float larguraJohn = alturaJohn * proporcao;

        if (john.x < larguraJohn / 2.0f) {
            john.x = larguraJohn / 2.0f;
        }

        if (john.x > larguraTela - larguraJohn / 2.0f) {
            john.x = larguraTela - larguraJohn / 2.0f;
        }

        if (john.y < alturaJohn) {
            john.y = alturaJohn;
        }

        if (john.y > alturaTela - 10.0f) {
            john.y = alturaTela - 10.0f;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexturePro(
            cenarios[cenarioAtual],
            (Rectangle) {
                0.0f,
                0.0f,
                (float)cenarios[cenarioAtual].width,
                (float)cenarios[cenarioAtual].height
            },
            (Rectangle) {
                0.0f,
                0.0f,
                larguraTela,
                alturaTela
            },
            (Vector2) {0.0f, 0.0f},
            0.0f,
            WHITE
        );

        if (
            mostrarColisoes &&
            (cenarioAtual == SALAO || cenarioAtual == BANHEIRO)
        ) {
            Texture2D texturaMascaraAtual = (cenarioAtual == SALAO)
                ? texturaMascaraSalao
                : texturaMascaraBanheiro;

            DrawTexturePro(
                texturaMascaraAtual,
                (Rectangle) {
                    0.0f,
                    0.0f,
                    (float)texturaMascaraAtual.width,
                    (float)texturaMascaraAtual.height
                },
                (Rectangle) {
                    0.0f,
                    0.0f,
                    larguraTela,
                    alturaTela
                },
                (Vector2) {0.0f, 0.0f},
                0.0f,
                (Color){26, 214, 123, 220}
            );
        }

        DrawTexturePro(
            atual,
            (Rectangle) {
                0.0f,
                0.0f,
                (float)atual.width,
                (float)atual.height
            },
            (Rectangle) {
                john.x - larguraJohn / 2.0f,
                john.y - alturaJohn,
                larguraJohn,
                alturaJohn
            },
            (Vector2) {0.0f, 0.0f},
            0.0f,
            WHITE
        );

        if (
            mostrarColisoes &&
            (cenarioAtual == SALAO || cenarioAtual == BANHEIRO)
        ) {
            Rectangle caixaJohn = CaixaColisaoJohn(john, alturaJohn);

            DrawRectangleRec(
                caixaJohn,
                (Color){0, 220, 255, 90}
            );

            DrawRectangleLinesEx(
                caixaJohn,
                2.0f,
                SKYBLUE
            );

            char infoMouse[160];

            snprintf(
                infoMouse,
                sizeof(infoMouse),
                "Mouse: X %.2f%% | Y %.2f%%",
                mouse.x / larguraTela * 100.0f,
                mouse.y / alturaTela * 100.0f
            );

            DrawRectangle(10, 10, 490, 62, (Color){0, 0, 0, 190});

            DrawText(
                infoMouse,
                20,
                18,
                20,
                YELLOW
            );

            DrawText(
                "F1: ocultar colisao | Clique: registrar ponto no terminal",
                20,
                44,
                16,
                RAYWHITE
            );

            DrawCircleV(mouse, 5.0f, YELLOW);
        }

        EndDrawing();
    }

    for (int i = 0; i < NUM_CENARIOS; i++) {
        UnloadTexture(cenarios[i]);
    }

    for (int d = 0; d < NUM_DIRECOES; d++) {
        for (int f = 0; f < NUM_FRAMES; f++) {
            UnloadTexture(animacoes[d][f]);
        }

        UnloadTexture(johnParado[d]);
    }

    UnloadTexture(texturaMascaraSalao);
    UnloadTexture(texturaMascaraBanheiro);

    UnloadImage(mascaraSalao);
    UnloadImage(mascaraBanheiro);

    CloseWindow();
    return 0;
}