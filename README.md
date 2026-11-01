# VHS — A Última Sessão

Jogo em C com Raylib, ambientado em uma locadora abandonada. Esta entrega integra as cinco salas, suas colisões e as passagens entre elas. As demais mecânicas serão retomadas depois, conforme a orientação da equipe.

## Executar

Na pasta do projeto:

```sh
make run
```

O executável atualizado fica em `build/vhs`. No VS Code, Ctrl+Shift+B compila; Terminal → Executar Tarefa → Rodar jogo inicia a aplicação.

| Comando | Ação |
|---|---|
| WASD ou setas | Andar; continuar na direção da porta para atravessá-la |
| F1 | Mostrar/ocultar máscaras, caixa dos pés, áreas de transição e chegadas |
| Clique com F1 ativo | Registrar coordenadas da imagem no terminal |
| F11 | Alternar tela cheia sem bordas |
| Esc | Encerrar |

## Salas e conexões

- Salão ↔ banheiro.
- Salão ↔ escritório.
- Salão ↔ estoque.
- Estoque ↔ sala de projeção.

Os cenários mantêm sua resolução original de 1672 × 941. Janela e tela cheia preservam a proporção; tamanhos diferentes usam barras pretas. O banheiro e sua máscara continuam sendo os arquivos entregues pelo colega. As máscaras do escritório, estoque e projeção foram acrescentadas nesta revisão.

## Compilar e testar

São necessários GCC, Make e Raylib. Neste computador, a Raylib 5.5 para Linux x86_64 está em `.deps/raylib`, obtida da [distribuição oficial](https://github.com/raysan5/raylib/releases/tag/5.5). A pasta deve conter `include/` e `lib/` diretamente. `.deps/` e `build/` ficam fora do Git. O makefile também aceita a instalação da equipe em `/usr/local` quando o pacote local não existe.

```sh
make              # compilar
make test         # ativos, colisões, entradas e rotas; requer Python 3 + Pillow
make sanitize     # colisões e rotas com AddressSanitizer/UBSan
make masks        # regenerar somente escritório, estoque e projeção; requer Pillow
```

O teste em C não precisa de janela. Para revisar visualmente, use `make run` e F1. Para capturar automaticamente os cinco ambientes, crie previamente uma pasta e execute:

```sh
./build/vhs --size 1672 941 --gallery /caminho/absoluto/da/pasta
```

Também estão disponíveis `--fullscreen` e `--frames N --capture /caminho/absoluto/imagem.png`.

## Organização

- `src/main.c`: janela, recursos, comandos, animação e integração.
- `src/world.c`: salas, portas, pontos de chegada e conversão de coordenadas.
- `src/collision.c`: colisão compartilhada para todas as máscaras.
- `src/scene_draw.c`: personagem e ocultação atrás dos móveis.
- `tools/masks.json`: geometria editável das três máscaras novas.
- `tests/`: verificações automatizadas de ativos, colisões e circulação.
- `docs/REVISAO.md`: checklist, evidências, testes e limites desta entrega.

A integração parte do commit `0aff630` do grupo. O código anterior e as mudanças locais foram preservados em backups antes da atualização. Os PNGs existentes não foram substituídos.
