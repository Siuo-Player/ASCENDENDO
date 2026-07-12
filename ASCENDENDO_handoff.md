# ASCENDENDO — Contexto Completo do Projecto (Handoff)

> Documento gerado para arrancar uma nova conversa sem repetir todo o histórico. Lê isto primeiro — tem tudo o que importa: arquitectura, decisões tomadas, bugs corrigidos (e porquê), regras de trabalho, estado actual, e o plano em falta.

---

## 1. Identidade do Projecto

**ASCENDENDO** — vertical precision platformer 2D, motor Vulkan próprio escrito de raiz em C++20.
**Autor:** Rafael Gomes Bernardo. **Colaboradores IA:** Claude (Anthropic, principal) + Gemini (Google, consultor secundário — **as sugestões do Gemini devem ser verificadas de forma independente**, ele não tem acesso ao histórico completo do projecto e já esteve parcialmente errado antes).
**Repo:** github.com/Siuo-Player/ASCENDENDO
**Mecânica central:** salto parabólico com inércia ("Commitment Jump") — ângulo fixo 60°, sem controlo aéreo, força controlada por carga (hold SPACE).

**Motivo de existir uma engine própria + pixel art simples:** optimização de espaço é um requisito permanente e deliberado — não é preguiça nem limitação, é a razão de ser do projecto. Qualquer feature nova deve ser avaliada também por este critério.

---

## 2. Stack Técnico

| Componente | Escolha |
|---|---|
| Linguagem | C++20 |
| Compilador | Clang++ (LLVM), `Target: x86_64-pc-windows-msvc` |
| Build | GNU Make, Git Bash/MSYS2 (Windows) |
| Gráfica | Vulkan 1.4 (explícito, sem abstracções) |
| Janela | GLFW 3.4 |
| Testes | doctest 2.5.0 (header-only) |
| Shaders | GLSL → SPIR-V via `glslc` |
| Fontes | stb_truetype.h (single-header) |
| Imagens | stb_image.h (single-header) |
| Optimização PNG | optipng (confirmado instalado via `winget install -e --id OptiPNG.OptiPNG`) |
| Validação de níveis | Python 3.x (`sim/` replica a física do C++ 1:1) |
| Hardware do Rafael | Windows, Intel Iris Xe (GPU integrada) |

**Nota crítica:** não tenho visto o `Makefile` actual do Rafael (só uma cópia desactualizada nos project files). `make shaders` reporta "Nothing to be done" para shaders novos (`text.*`, `sprite.*`) — o alvo não faz glob automático. Solução actual: compilar à mão com `glslc` sempre que se adiciona um shader novo. Isto devia ser corrigido no Makefile real, mas não posso editá-lo sem o ver.

---

## 3. Arquitectura

```
ASCENDENDO/
├── Game/
│   ├── Core/        Config.h (constantes globais), CampaignID.h (hash FNV-1a)
│   ├── Graphics/     ver secção 3.1 (3 pipelines Vulkan separadas)
│   ├── Logic/        Physics, Player, Level, InputManager, ReplayManager, RunHistory.h
│   └── Assets/       Shaders/, Fonts/, Sprites/ (+Source/ com .pixil originais), Levels/
├── Tests/            Unit/ (sem GPU) e Integration/ (GPU real necessária)
├── Development/       dev_log.txt (append-only), AI_Validation/ (validador Python + sim/), Runs/ (runs.csv)
├── scripts/          pre-commit.sh, pre-push.sh
├── main.cpp, reorganize.py, Makefile, README.md
```

### 3.1 — Três pipelines Vulkan, deliberadamente separadas

Decisão arquitectural chave: em vez de expandir a `Pipeline` original (retângulos sólidos) para suportar texturas, criei **pipelines dedicadas** por tipo de conteúdo. Razão: zero risco para código já testado — cada pipeline nova é 100% aditiva.

| Pipeline | Ficheiros | Sampler | Uso |
|---|---|---|---|
| `Pipeline` (original) | Pipeline.h/.cpp, base.vert/frag | — (cor sólida via push constants) | Plataformas, FLAG, UI decorativa |
| `TextPipeline` | TextPipeline.h/.cpp, FontRenderer.h/.cpp, text.vert/frag | LINEAR | Texto TTF real (Orbitron, atlas R8_UNORM 512×512, baked a 48px) |
| `SpritePipeline` | SpritePipeline.h/.cpp, SpriteRenderer.h/.cpp, sprite.vert/frag | **NEAREST** (crítico p/ pixel art — LINEAR borraria) | Jogador (`personagem.png`, 32×32 RGBA) |

Todas as três partilham o mesmo `RenderPass` e alternam dentro do mesmo command buffer via `vkCmdBindPipeline` (viewport/scissor dinâmicos mantêm-se válidos entre pipelines compatíveis). Cada uma tem o seu próprio `VkDescriptorSetLayout` + push constants struct (structs só com `float`/`float[N]`, para o layout em C++ corresponder exactamente ao GLSL sem padding escondido).

**Fallback gracioso:** `Renderer::attachText()` e `Renderer::attachSprite()` são setters opcionais chamados por `main.cpp` depois de tudo inicializar. Se falharem (shader `.spv` ou asset em falta), o jogo cai automaticamente em: `BitmapFont.h` (fonte 5×5 pixelada) para texto, rectângulo sólido para o jogador. Nunca crasha por falta de um asset.

### 3.2 — Sistemas principais

- **PhysicsWorld**: fixed timestep 60Hz (`FIXED_STEP=1/60s`), `GRAVITY=-980`. Colisão MTV (Minimum Translation Vector) — topo one-way, lados sólidos com bounce elástico -30%.
- **Player**: `MOVE_SPEED=140`, `MIN_JUMP=250`, `MAX_JUMP=600`, `CHARGE_TIME=0.4s`. Ângulo fixo 60°: `VX = F·cos60° = F·0.5`, `VY = F·sin60° = F·0.866`. `facingDirection` (±1) usado para flip do sprite.
- **Level**: `appendFromFile()` carrega um `.lvl` e devolve **sempre** `offsetY + LOGICAL_HEIGHT` (360px) — nunca uma altura variável (isto foi um bug grave, ver secção 5). Chunks empilhados verticalmente por streaming.
- **Camera**: `follow()` com Lerp vertical, jogador a 35% da base do ecrã.
- **GameState** (`enum class`): `PLAYING → PAUSED` (ESC, sem terminar a run) `→ CREDITS` (FLAG tocada ou botão) `→ MENU` (fim-de-run, 3 opções: Começar/Créditos/Sair). PAUSED também tem 3 opções (Continuar/Créditos/Sair) e desenha o mundo **congelado e visível** por baixo dum overlay escuro (não um ecrã opaco separado).
- **Timer**: acumula em `elapsedTime` só durante `PLAYING` (dt real de chrono, desligado da simulação fixed-step — não afecta determinismo). HUD desenhado no canto superior direito durante o jogo.
- **RunHistory.h**: ao tocar a FLAG, grava uma linha CSV em `Development/Runs/runs.csv` (timestamp, nome da campanha, ID, tempo). Header-only.
- **CampaignID.h**: hash FNV-1a 64-bit determinístico sobre `campaign.txt` + cada `.lvl` referenciado, na ordem. Resolve a questão "como diferenciar campanhas com o mesmo nome" — qualquer byte alterado muda o ID. **Nota:** esta abordagem foi decidida sem confirmação prévia explícita do Rafael (o README sinaliza isto) — vale a pena confirmar se ele prefere outra estratégia antes de expandir em cima disto.

### 3.3 — Sim/ (Python) — geração e validação de níveis

`Development/AI_Validation/sim/` replica a física do motor C++ **exactamente** (mesmas fórmulas, mesmo MTV) para gerar e validar níveis por simulação real, não só análise geométrica:
- `engine.py`: `Body`, `physics_step`, `resolve_collision` (espelha `Physics.cpp`/`Level.cpp`)
- `levelgen.py` (v4): gera plataformas simulando o salto real. `ascending_x_at()` calcula onde o corpo está **durante a subida** para garantir que a borda da próxima plataforma não bate lateralmente nele (bug histórico, ver secção 5). Clearance de 40px.
- `ai_validator.py`: valida `--campaign` via BFS com física real (`V_MAX=600`, `TOLERANCE=0.90`). É chamado por `test_campaign.cpp` via `std::system()` — portanto corre dentro do `make tests` e bloqueia o pre-commit se um nível for impossível.

**Routing automático de níveis** (`reorganize.py`, tabela útil para debug): válido + listado em `campaign.txt` → `Game/Assets/Levels/`; válido + não listado → `.../Unused/`; inválido → `.../NaoValidados/`.

---

## 4. Testes — Estado Actual

**81 test cases, 274 assertions, 0 falhas** (último `make tests-verbose` confirmado, commit `05f86e2`).

Distribuição por ficheiro (Unit = sem GPU, Integration = GPU real necessária):
- Unit: placeholder, vulkan_context, input, physics, player, replay, camera, level (16), campaign (chama ai_validator), **campaign_id (7 testes)**, **run_history (2 testes)**
- Integration: vulkan_init, window, swapchain, render_pass, renderer, pipeline, **text_pipeline (2)**, **sprite_pipeline (2)**

**Regra de ouro do projecto:** testes antigos nunca são alterados para acomodar código novo. TDD — teste escrito antes da implementação. Pre-commit bloqueia commits com testes a falhar; pre-push corre `ai_validator.py --campaign`.

---

## 5. Bugs Corrigidos (histórico de aprendizagem — não repetir)

Cada um destes já custou tempo real a diagnosticar — o raciocínio está aqui para não ser refeito:

1. **`duplicate symbol` no linker**: `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` só pode existir num único `.cpp` de todo o projecto (`Tests/test_runner.cpp`). Qualquer novo ficheiro de teste inclui só `#include "doctest/doctest.h"`, sem a macro.
2. **`UnicodeEncodeError` no Windows (cp1252)**: emojis em scripts Python (`✅`❌`) crasham no terminal Windows. Todos os scripts Python do projecto usam `sys.stdout.reconfigure(encoding='utf-8')` + `[OK]`/`[ERRO]` ASCII, nunca emoji.
3. **Streaming de níveis quebrado**: `appendFromFile` costumava devolver `highestY+50` (variável, dependia do conteúdo). O trigger de streaming assume blocos de altura fixa. Corrigido para devolver **sempre** `offsetY+LOGICAL_HEIGHT`.
4. **`LOGICAL_WIDTH`/`LOGICAL_HEIGHT` trocados** nos validadores Python (360×640 em vez de 640×360) — bloqueava silenciosamente plataformas de largura total.
5. **Colisão lateral na subida do salto**: corpo a saltar para um lado entrava na zona X da plataforma alvo ainda a subir → motor detectava colisão lateral (não topo) → ricochete. Corrigido com `ascending_x_at()` no `levelgen.py`, garante 40px de clearance na borda que o corpo atravessa a subir.
6. **FLAG com "cor estranha" (castanho/oliva)**: zona de touchdown semi-transparente (alpha 0.18) misturava-se com o fundo azul-escuro. Removida — fica só mastro+pano.
7. **Timer HUD "garbled"**: **não era bug de fonte nem UV mapping.** O baseline do texto estava tão perto do topo do ecrã lógico (360) que o topo de cada glyph era cortado pelo clipping de NDC/viewport. Confirmado por análise geométrica (CREDITS usa a mesma função e renderiza perfeitamente). Corrigido reposicionando bem abaixo do limite.
8. **Espaçamento em CREDITOS**: labels pequenos seguidos de nomes grandes com baselines demasiado próximos = sobreposição visual. Corrigido recalculando TODO o layout (não só encolher texto).
9. **Testes flaky no Windows**: `std::filesystem::remove_all(dir)` (a variante que lança excepção) podia falhar porque o Windows mantém um ficheiro momentaneamente "bloqueado" mesmo depois do `ofstream` fechar. Corrigido: todas as chamadas usam a variante `(dir, ec)` não-throwing. Pastas de teste consolidadas em `build/test_tmp/` (já gitignored, limpo por `make clean`) em vez de soltas na raiz.
10. **Sprite não carregava ("player.png" vs "personagem.png")**: eu tinha um mapeamento de sinónimos frágil (`"ascended"→"player"`) que só funcionava para o nome exacto que eu tinha adivinhado. Corrigido removendo a adivinhação — `reorganize.py` converte sempre genérico (nome.pixil→nome.png), e `main.cpp` é agora a fonte explícita da verdade sobre qual PNG é o jogador.
11. **`.pixil` corrompido**: ficheiros exportados do Pixilart tinham uma sequência de "lixo" tipo keyboard-mash (`98kjasdnasd983/24kasdjasd`) inserida entre `image/png` e `base64,` no data URI. Tratado como corrupção de export, removido com regex antes de descodificar.
12. **Formato de sprite**: testei PNG normal vs indexado (paleta exacta) — sem optimizar, o indexado ficava **maior**. Depois de `optipng -o7` em ambos, convergem para o mesmo tamanho óptimo. Conclusão prática: não vale a pena escolher formato à mão, basta sempre correr `optipng` a seguir à conversão (agora automático no `reorganize.py`, com fallback gracioso se não estiver instalado, e reoptimização retroactiva de todos os sprites a cada execução).

---

## 6. Regras de Trabalho (Rafael → Claude)

Estas são preferências explícitas e repetidas — seguir sem precisar de as relembrar:

- **Confirmar antes de implementar decisões arquitecturais.** "São apenas ideias, antes de as implementar deves me perguntar." Isto aplicou-se explicitamente à Fase 9 (Editor) — não construir às cegas quando o scope está genuinamente por definir.
- **Validação empírica sempre que possível**, não só análise/raciocínio. "No fim confirma simulando." Nesta sessão isto significou: instalar `libvulkan-dev`, `glslc`, headers GLFW reais neste sandbox via `apt`, e compilar+ligar o código C++/Vulkan novo contra stubs que espelham as APIs reais do projecto, ANTES de entregar. Isto apanhou bugs reais (includes em falta, assinaturas erradas) antes da entrega. **Este é agora o workflow padrão** para qualquer código C++/Vulkan novo.
- **Nunca alterar assertions de testes existentes** sem instrução explícita.
- **TDD estrito**: testes antes da implementação, pre-commit bloqueia falhas.
- **Entrega de ficheiros deve ser sempre verificada.** Houve MÚLTIPLAS falhas nesta sessão em que eu disse "entreguei" sem ter chamado `present_files` correctamente, ou os ficheiros não chegaram. Regra agora: sempre confirmar explicitamente a contagem de ficheiros entregues, e nunca assumir que uma entrega anterior chegou sem confirmação do Rafael.
- **Optimização de espaço é requisito permanente e cumulativo** (secção 10 do README) — motivo de raiz do projecto (engine própria + pixel art = optimizar espaço).
- **Batch de entrega no fim da sessão** preferido a entregas fragmentadas.
- **Git**: incluir `git add/commit/push` com mensagens detalhadas depois de cada lote confirmado a passar.
- **dev_log.txt append-only** — nunca editar entradas antigas, só acrescentar.
- **Nomes de ficheiros de conteúdo** (assets, níveis) tendem a português quando natural (`personagem.pixil`, não `character.pixil`); nomes de código/classes em inglês técnico convencional (`CampaignID.h`).

---

## 7. Estado Actual (confirmado, commit `05f86e2`)

- ✅ 81/81 testes a passar, 274 assertions
- ✅ Campanha completa jogável: `inicio.lvl → zigzag.lvl → precipicio.lvl` (FLAG no fim)
- ✅ Sprite do jogador real (`personagem.png`, 32×32, de `.pixil` convertido automaticamente)
- ✅ Texto TTF real (Orbitron) em CREDITOS/MENU/PAUSA + timer HUD
- ✅ Sistema de pausa (ESC), menu de fim-de-run (3 opções), créditos acessíveis de ambos
- ✅ Histórico de runs (`Development/Runs/runs.csv`) com ID determinístico de campanha
- ✅ `.gitignore` protegido contra pastas temporárias de teste (`/tmp_*`)
- ✅ Push feito para `github.com/Siuo-Player/ASCENDENDO`, working tree limpo

---

## 8. O QUE FALTA — Plano Explícito, por Ordem

### 8.1 — Imediato / pendente de decisão do Rafael
Nada bloqueado tecnicamente neste momento. Próximo passo é a escolha do Rafael entre:

### 8.2 — Fase 9: Editor de Níveis Visual *(scope AINDA por definir — não começar sem responder a estas perguntas)*
Visão: colocar/redimensionar plataformas com rato, validação IA automática, feedback visual de nível impossível, mais tarde escolha/criação/edição de sprites (chão/plataforma/decoração) com regras de troca — mas isto último é explicitamente **posterior ao jogo estar jogável** (já está).
Perguntas em aberto antes de implementar:
- Como colocar/redimensionar com o rato (não existe input de rato em `InputManager` — precisa de ser construído de raiz)
- Feedback de validação: cor sobre a plataforma? painel lateral? outro?
- Validação corre a cada solta do rato, ou só ao guardar? (implica modelo de threading para não bloquear o render loop)
- Guarda directamente para `.lvl`, ou formato intermédio?

#### 8.2.1 — Sistema de Auto-Tiling (expansão pedida por Rafael, **baixa prioridade, "pode ser no fim"**)

Rafael já fez um protótipo manual da técnica: tile base `pedra16.pixil` (16×16, pedra/rocha — pensado para plataformas, possivelmente paredes, possivelmente fundo com paralaxe) combinado via **rotação + espelhamento horizontal/vertical** em `pedra64.pixil` (64×64 composto). Os dois ficheiros já foram enviados como exemplo da técnica (não precisam de conversão agora — são só referência de design).

Visão a expandir no futuro: o que jogos com editor de níveis costumam ter é **auto-tiling** (também conhecido como *Wang tiles* / *blob tiling* / o sistema clássico do RPG Maker e do Tiled Map Editor) — regras que escolhem automaticamente qual variante de sprite colocar consoante os vizinhos (bitmask de "há chão à esquerda? à direita? em cima? em baixo?"), para que um bloco de terreno pareça orgânico e contínuo em vez de blocos repetidos com costuras visíveis. Isto exige:
- Sprites desenhados **sem margens**, pensados para encaixar nas bordas uns com os outros (Rafael já está a desenhar com isto em mente)
- Um conjunto de variantes por tile lógico (tipicamente 16 ou 47 variantes num sistema de bitmask completo, dependendo da complexidade desejada — cantos, bordas, interior, ilhas isoladas)
- Lógica no motor (ou no editor da Fase 9) que escolhe a variante certa a partir da vizinhança de cada célula do nível

Não implementar sem discussão — é aqui deixado como visão expandida e registada, para retomar quando a Fase 9 (ou uma iteração dela dedicada só a tiles) chegar à vez.

### 8.3 — Fase 10: Distribuição / "Release Build" *(definida em detalhe, não implementada — "um dos últimos passos")*
5 regras obrigatórias já documentadas no README com notas técnicas:
1. Sem terminal visível — flag de linker por confirmar (`-mwindows` estilo MinGW vs `/SUBSYSTEM:WINDOWS` estilo MSVC — o projecto usa Clang a atingir MSVC ABI mas compila via Git Bash, ambíguo até testar)
2. Pacote portable, sandboxing total, nada fora da própria pasta
3. Paths relativos/locais só — nunca `AppData`/`Documentos`/Registo
4. Falha graciosa: se faltar suporte Vulkan, `MessageBoxW` nativa + fecho limpo. Ponto de entrada natural já identificado: `VulkanContext::init()` já devolve `bool`, mas `main.cpp` actual **não verifica esse retorno** — é aqui que entra a checagem. Precisa de `#ifdef _WIN32` para não partir o build Linux.
5. README reestruturado: secções separadas "Para Desenvolvedores" vs "Para Jogadores"

### 8.4 — Manutenção pendente (baixa prioridade, sem bloquear nada)
- Ver o `Makefile` real do Rafael e corrigir o alvo `shaders` para fazer glob automático (actualmente lista fixa, não apanha `.vert`/`.frag` novos)
- Confirmar que `optipng` está a ser encontrado correctamente pelo `reorganize.py` no ambiente real (já confirmado que sim, `v7.9.1` instalado via `winget`)
- Considerar atlas único de sprites (spritesheet) quando houver mais do que 1-2 sprites — reduz overhead de descriptor sets

### 8.5 — Sem prioridade fixa (registo, não esquecer)
- Sistema de dificuldade por nível/campanha (o `ai_validator`/`verify_campaign.py` já calculam robustez por salto — falta agregar e expor ao jogador)
- Múltiplos modos/mapas/campanhas seleccionáveis (depende de Fase 9)

---

## 9. Comandos Rápidos (referência)

```bash
# Depois de qualquer ficheiro novo/alterado ser colocado na raiz:
python reorganize.py          # organiza, valida niveis, converte .pixil->PNG+optipng

# Shaders novos (make shaders normalmente NAO apanha automaticamente):
glslc Game/Assets/Shaders/<nome>.vert -o Game/Assets/Shaders/<nome>.vert.spv
glslc Game/Assets/Shaders/<nome>.frag -o Game/Assets/Shaders/<nome>.frag.spv

# Testar:
make clean && make tests-verbose -j8 > Tests/log_testes.txt 2>&1
cat Tests/log_testes.txt

# Jogar:
make game && ./build/game.exe

# Git (só depois de confirmar testes a verde):
git add . && git commit -m "mensagem detalhada" && git push
```

---

## 10. Convenções de Versão

`vX.Y[Z]` — X=fase principal, Y=sub-passo, Z=fix incremental. Cada ficheiro tem `@version` no cabeçalho reflectindo o seu próprio histórico incremental (podem não bater certo com o "marco do projecto" global — isso é esperado, ver nota no README secção 9). Marco actual do projecto: **8.3.1**.

---

## 11. Os 20 Ficheiros Mais Importantes (para anexar a uma conversa nova)

Este documento resume tudo, mas os ficheiros REAIS continuam a ser a fonte da verdade para qualquer edição precisa. Lista ordenada por valor informativo/token — os primeiros 10 são o mínimo essencial; os segundos 10 aprofundam.

**Top 10 (essencial):**
1. `README.md` — referência mestra, roadmap, tudo
2. `Development/project_structure.txt` — árvore de ficheiros real, auto-gerada
3. `Makefile` — **eu não tenho a versão real actual** (só uma cópia desactualizada); isto resolve um ponto cego activo (`make shaders` não funciona bem, não posso corrigir sem ver o ficheiro real)
4. `Game/Core/Config.h` — todas as constantes de gameplay num sítio, pequeno e denso
5. `main.cpp` — loop principal, máquina de estados, liga tudo
6. `Game/Graphics/Renderer.cpp` — a lógica de renderização mais complexa, mostra as 3 pipelines em prática
7. `Game/Graphics/Renderer.h`
8. `Game/Logic/Level.cpp` — carregamento de níveis + colisão MTV
9. `Game/Logic/Level.h`
10. `Game/Logic/Player.cpp` — mecânica central (Commitment Jump)

**Próximos 10 (aprofundam):**
11. `Game/Logic/Player.h`
12. `Game/Logic/Physics.cpp`
13. `Game/Logic/Physics.h`
14. `reorganize.py` — toda a lógica do pipeline de assets/organização
15. `Game/Core/CampaignID.h`
16. `Game/Logic/RunHistory.h`
17. `Development/AI_Validation/ai_validator.py`
18. `Tests/Unit/test_level.cpp` — exemplo representativo das convenções de teste/TDD
19. `Game/Graphics/SpriteRenderer.h` — padrão arquitectural mais recente (pipeline dedicada)
20. `Development/dev_log.txt` — histórico completo; prioridade mais baixa porque este handoff já condensa as aprendizagens principais, mas continua a ser a referência definitiva para o que não estiver aqui

---

*Fim do handoff. Qualquer detalhe mais fino que não esteja aqui provavelmente não é crítico — mas se precisares de recuperar algo específico da conversa antiga, os ficheiros reais do projecto (que o Rafael tem no disco) são sempre a fonte da verdade, não a memória desta conversa.*
