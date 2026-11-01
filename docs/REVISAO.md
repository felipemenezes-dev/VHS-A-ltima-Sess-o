# VHS — revisão das salas, entradas e saídas

Data: 01/11/2026. Base integrada: `0aff630` — “Adicionada a colisao do banheiro”. Branch local: `revisao/integracao-cenarios`.

## Escopo confirmado

A última orientação do usuário restringiu esta etapa às salas, suas entradas/saídas e testes. Menus, inventário, fitas, inimigos, eventos e finais ficam para depois. O trabalho de gameplay já iniciado foi guardado separadamente e removido da versão ativa desta entrega.

As cinco artes já existiam no repositório atualizado. Portanto, esta entrega não reivindica a criação das imagens. Foram finalizadas a circulação e as colisões que faltavam. A imagem e a máscara do banheiro pertencem ao colega e foram preservadas, assim como a máscara do salão.

## Atualização e preservação

A cópia local começou em `8bbdd40`, anterior à quinta sala e às máscaras do grupo. Após a solicitação de sincronização, o projeto foi atualizado até `0aff630`, incorporando os commits `4e217ae`, `c7b52ea`, `42bb3df` e `0aff630`. As alterações da revisão foram adaptadas sobre essa base. Consultas posteriores ao remoto continuaram apontando para `0aff630`.

README e configuração local de compilação/VS Code já tinham alterações; elas foram preservadas em backup. A execução antiga foi registrada em capturas de referência. Os problemas daquela referência não são atribuídos à versão mais nova do colega: a nova base já continha cinco salas e colisões no salão e banheiro.

Nenhum push foi feito. O trabalho ficou disponível na pasta local do projeto, com um patch separado para revisão e integração.

## Checklist

| Item | Estado encontrado na base atualizada | Evidência / problema | Alteração | Testes executados | Estado final |
|---|---|---|---|---|---|
| Salão principal | Parcial | Arte e máscara do grupo; três passagens | Uso da colisão compartilhada e ajuste das áreas/pontos de transição | Navegação, obstáculos, idas e voltas, captura normal/F1 | Concluído e validado no escopo de circulação |
| Banheiro | Parcial | Arte e máscara recém-entregues pelo colega | Integração genérica e chegada fora da área de retorno; PNGs intactos | Entrada, saída, colisão e comparação dos arquivos com o Git | Concluído e validado na integração; autoria preservada |
| Escritório | Parcial | Arte existente, sem máscara própria carregada | Máscara nova de paredes/móveis, entrada e retorno ao salão | Navegação, obstáculos, passagem nos dois sentidos e inspeção visual | Concluído e validado |
| Estoque | Parcial | Arte e conexões existentes, sem máscara própria | Máscara nova, circulação até salão e projeção | Navegação, cantos, corredores e ambas as conexões | Concluído e validado |
| Projeção | Parcial | Arte já integrada pelo grupo, sem máscara própria | Máscara nova e passagem de volta ao estoque | Navegação, móveis e ida/volta | Concluído e validado |
| Colisão compartilhada | Parcial | Apenas duas máscaras utilizadas na base | Todas as cinco; caixa inteira dos pés; consulta de pixels preparada ao carregar; movimento em passos de até 1 pixel | Paredes/cantos, linha sintética de 1 pixel, movimento rápido, diagonal e deslizamento | Concluído e validado |
| Entradas e saídas | Parcial | Coordenadas dependentes da janela e risco de sobreposição com retorno | Coordenadas da arte, destino livre, direção de saída e intervalo de 0,25 s | 96 passagens em rotas completas + 40 aproximações com quadros variados | Concluído e validado |
| Escala e personagem | Parcial | Renderização ligada às dimensões da tela; margens transparentes diferentes no sprite parado | Coordenadas fixas, proporção preservada e recorte transparente somente ao carregar | Sete resoluções nas conversões; janela, 4:3, tela cheia e quatro direções na execução | Concluído e validado |
| Depuração | Parcial | F1 e registro de coordenadas já existentes | Mantidos; acrescentados gatilhos, chegadas e barreiras das portas | F1, clique e inspeção das cinco sobreposições | Concluído e validado |
| Inicialização e recursos | Parcial | Consulta de monitor antes da criação da janela | Inicializar janela primeiro; verificar recursos e pontos de entrada; liberar recursos ao sair | Compilação, abertura, encerramento e falha controlada por máscara ausente | Concluído e validado |
| Demais mecânicas | Ausentes da versão ativa da base | Fora da prioridade estabelecida pela última orientação | Rascunho anterior guardado para retomada | Não fazem parte da validação desta entrega | Adiadas pelo usuário |

## Conexões e chegadas

As coordenadas abaixo indicam os pés de John na arte original de 1672 × 941. A troca acontece ao caminhar para fora pela área da porta; não exige uma tecla de interação.

| Origem | Destino | Chegada no destino |
|---|---|---|
| Salão | Banheiro | 1320, 620 |
| Banheiro | Salão | 185, 520 |
| Salão | Escritório | 835, 704 |
| Escritório | Salão | 500, 290 |
| Salão | Estoque | 845, 727 |
| Estoque | Salão | 1174, 293 |
| Estoque | Projeção | 825, 704 |
| Projeção | Estoque | 845, 305 |

O limite do mundo é independente da resolução da janela. As aberturas dos PNGs do grupo permanecem intactas; pequenas barreiras de portal no código impedem sair da arte antes de a transição ocorrer. As caixas dos pés e as barreiras podem ser inspecionadas com F1.

## Arquivos desta revisão

- Novos: `assets/cenarios/colisoes_escritorio.png`, `colisoes_estoque.png`, `colisoes_sala_projecao.png`.
- Todas as máscaras: 1672 × 941, RGBA, fundo transparente e pixels bloqueados brancos com alfa 255; nenhum pixel semitransparente.
- Integração: `src/main.c`, `src/world.[ch]`, `src/collision.[ch]`, `src/scene_draw.[ch]`.
- Compilação: `makefile`, mantendo suporte ao pacote local e à instalação `/usr/local` do grupo.
- Ferramentas/testes: `tools/build_masks.py`, `tools/masks.json`, `tests/navigation.h`, `tests/test_rooms.c`, `tests/test_assets.py`.
- Documentação: `README.md` e este checklist.
- As cinco artes, os sprites e as duas máscaras vindas do grupo não foram alterados.

## Resultados executados

- Compilação com C99, `-Wall -Wextra -Wpedantic`: aprovada, sem avisos.
- Ativos: 140 verificações aprovadas, incluindo dimensões, integridade, alfa das máscaras e frames do personagem.
- Colisão e circulação: 72.106 verificações aprovadas, zero falhas.
- 10.573 abordagens de paredes/cantos em oito direções, com deslocamentos de até 512 pixels para detectar atravessamento; 12.500 deslocamentos variados adicionais.
- Rotas conectadas: três voltas por configuração de tempo, oito passagens por volta, quatro configurações; 96 passagens no total, começando nas chegadas reais.
- Aproximações das portas: 40 casos adicionais, com passos entre 1/144 s e 0,25 s.
- AddressSanitizer e UndefinedBehaviorSanitizer: testes de colisão/circulação aprovados, sem diagnóstico de erro ou vazamento.
- Conversões de coordenadas: 640×360, 800×600, 1024×768, 1280×720, 1672×941, 1920×1080 e 2560×1080.
- Execução gráfica em Xvfb: carregamento e captura das cinco salas, normal e com F1; inspeção visual de alinhamento e proporção.
- Teclado real via X11: percurso entre as salas, direção e animação, colisões contra obstáculos e retorno pelas portas. F1, clique, redimensionamento, F11 e Esc conferidos na aplicação.

Comandos principais:

```sh
make -B
make test
make sanitize
make run
```

## Limites e continuação

A validação cobre a exploração entre ambientes. Não representa conclusão do jogo narrativo: interações, itens, inimigo, áudio, menus e finais serão tratados depois. A revisão acadêmica completa também fica fora desta etapa reduzida.

Os testes gráficos ocorreram em sessão X11 isolada, com Raylib 5.5/Linux. Outras plataformas não foram testadas. Os sanitizadores foram aplicados ao núcleo de colisões e rotas; não constituem auditoria das bibliotecas gráficas externas. As máscaras existentes do colega foram utilizadas e testadas sem redesenho.

Para mudanças futuras de arte, atualizar a geometria da máscara correspondente e repetir os testes de entrada/saída. A geração em `make masks` alcança somente escritório, estoque e projeção.
