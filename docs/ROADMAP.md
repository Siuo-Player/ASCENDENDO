# Roadmap de desenvolvimento

## Estado de referência

`main` contém a tranche de robustez/build/CI da 9.4 integrada na PR #7 (`10cceeb`). A branch atual, `feat/9-4-editor-visual-integration`, desenvolve a integração visual da 9.4 e contém uma tranche intermédia ainda não integrada.

Concluído:

- Fases 1–8: motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: rato + conversão window/logical + menus clicáveis.
- 9.3: `GameState::EDITOR`, acesso por menu/tecla, câmera livre e grelha.
- bootstrap GLFW/Vulkan robusto + cleanup de falhas parciais.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.
- 9.4 tranche 2: `EditorInteractionController`, cursor logical→world, hit-test, STAMP/DRAG, seleção, movimento e delete.
- 9.4 tranche 3: validação da dimensão mínima antes do snap, Makefile Windows/Linux, CI headless Vulkan determinístico, validação da campanha e documentação de CI.

Documentação arquitetural normativa:

- `docs/ARCHITECTURE.md` — direção de arquitetura e fronteiras entre runtime, editor, presentation e services.
- `docs/TECH_DEBT.md` — problemas classificados por prioridade e critérios de saída.

## Fase 9 — Editor de níveis

### 9.1 ✅ — Controlos

Concluída. Intenção lógica, rebind e persistência existem.

### 9.2 ✅ — Input de rato e viewport

Concluída. Cursor, botões, `windowToLogical()` e hit-test existem.

### 9.3 ✅ — Estado EDITOR

Concluída. O editor abre dentro do jogo, com câmera livre e grelha.

### 9.4 ▶ — Manipulação visual de entidades

A integração está a ser feita em tranches para manter cada PR verificável.

#### 9.4.a ✅ — Modelo e interação determinística

Concluído:

- `LevelEditorDocument` com grid/snap, bounds, spawn e FLAG;
- `EditorInteractionController` com STAMP, DRAG, seleção, movimento e delete;
- `EditorSession` a ligar input, camera, key bindings e controller;
- tamanho MEDIUM por defeito;
- `G` alterna STAMP/DRAG;
- `[` e `]` percorrem SMALL/MEDIUM/LARGE;
- clique esquerdo cria ou move;
- botão direito cancela;
- Delete/Backspace remove;
- preview determinístico e invisível quando fora do canvas válido;
- testes de input → controller → document.

#### 9.4.b ▶ — Renderização e feedback visual

Em curso na branch `feat/9-4-editor-visual-integration`:

- desenhar `LevelEditorDocument` no espaço de mundo;
- preview do preset ativo;
- seleção/hover visual;
- feedback claro de STAMP/DRAG/movimento;
- HUD de ferramenta/tamanho;
- ligação visual sem duplicar a lógica de edição;
- validação manual do fluxo real.

A PR #8 está aberta para esta tranche e só deve ser integrada quando o critério visual estiver cumprido.

## Gate 9.4.5 — Consolidação arquitetural antes de Save

**Novo gate obrigatório.** Não iniciar `9.5 Save + Validar` enquanto os problemas críticos de arquitetura identificados em `docs/TECH_DEBT.md` não tiverem sido tratados ou explicitamente desclassificados.

Prioridade:

1. extrair a máquina de estados/aplicação para reduzir o `main.cpp`;
2. introduzir uma fronteira `Game/Editor → RenderSnapshot → Renderer`;
3. migrar gameplay de teclas físicas para `GameAction`/`KeyBindings`;
4. limitar recuperação do fixed timestep após frames muito longos;
5. consolidar `LevelData` como modelo comum entre parser/runtime/editor;
6. resolver paths de runtime independentes do current working directory;
7. validar graphics/present queues e capacidades Vulkan corretamente;
8. adicionar Windows build/tests ao CI;
9. executar ASan/UBSan no CI;
10. garantir que `make game` é validado pelo CI;
11. introduzir invariantes/property tests onde tragam valor;
12. iniciar a limpeza de placeholders e documentação histórica redundante.

O objetivo deste gate não é reescrever o motor: é impedir que as próximas features criem um `main.cpp`/`Renderer.cpp` ainda mais acoplado.

## Fase 9.5 — Guardar + validar

Só começa depois do Gate 9.4.5.

- introduzir versão explícita no formato `.lvl`;
- extrair `LevelData` declarativo sem Vulkan/GLFW;
- serializar o documento para `.lvl`;
- preservar níveis em `NaoValidados/`;
- executar validação em background;
- permitir sair enquanto a validação corre;
- notificar resultado;
- **qualquer mapa importado ou descarregado da Internet tem de ser novamente validado pelo próprio EXE antes de poder ser jogado**;
- um mapa só é marcado como jogável depois de a validação local passar.

## Fase 9.6 — Editor de campanha

- lista de níveis em estilo playlist;
- reordenar a campanha;
- abrir um nível diretamente no `GameState::EDITOR` através de uma transição de estado;
- persistir apenas `campaign.txt` enquanto não existir metadata adicional necessária;
- não mover automaticamente ficheiros entre `Levels`, `Unused` e `NaoValidados`.

## Fase 10 — Hardening do runtime e ferramentas

Antes de partilha online, fechar o produto localmente:

- Windows CI completo;
- build/link do jogo no CI;
- ASan/UBSan;
- Vulkan device/queue/swapchain validation robusta;
- RAII e ownership dos wrappers Vulkan;
- `AssetResolver`/AssetManager quando a quantidade de assets justificar;
- separar configuração por domínio;
- `undo/redo` no editor através de comandos transacionais;
- testes de parser malformado, viewport, física e editor;
- property-based tests onde existam invariantes fortes;
- limpar `.gitkeep`, placeholders e artefactos de runtime versionados.

## Fase 11 — Partilha e biblioteca de mapas

A conectividade é opcional; o jogo base continua totalmente funcional offline.

### 11.1 — Export/import local

- botão **Exportar/Partilhar**;
- botão **Importar mapa**;
- pacote compacto e declarativo;
- pacote independente do repositório de desenvolvimento;
- extração para área temporária/controlada;
- validação local obrigatória pelo EXE;
- mapas inválidos nunca entram na campanha jogável.

### 11.2 — Biblioteca online

Site dedicado para:

- upload de mapas/pacotes;
- download de mapas/pacotes;
- páginas de mapas e metadados;
- eventualmente contas, autores, favoritos, versões e pesquisa.

A primeira versão pode ser HTTP(S) com upload/download. **WebSockets não são requisito inicial.**

### 11.3 — Partilha direta entre computadores

Possibilidade futura e separada do site:

- envio direto de um pacote entre dois computadores;
- só introduzir WebSockets/WebRTC/etc. quando existir uma necessidade concreta de comunicação bidirecional em tempo real.

## Fase 12 — Release / Portable Build

Objetivo final:

> um executável Windows x64 que possa ser copiado para outro computador dentro dos requisitos mínimos e executado sem instalar o ambiente de desenvolvimento.

Requisitos:

- `.exe` portable;
- assets e DLLs necessárias na própria distribuição;
- sem consola visível na versão final;
- sem paths dependentes do current working directory;
- sem downloads obrigatórios para iniciar o jogo;
- deteção amigável de requisitos ausentes;
- tentar funcionar no hardware mais fraco que suporte corretamente Vulkan;
- Vulkan é requisito gráfico real; máquinas sem Vulkan não são prometidas como suportadas.

“Qualquer computador” significa, na prática, **qualquer Windows x64 dentro dos requisitos mínimos definidos pelo projeto**.

## Regra de segurança de mapas

A autoridade final de jogabilidade é o executável do próprio jogo.

Nem:

- `reorganize.py`;
- o servidor do site;
- nem um campo `validated=true` enviado por outro computador

pode substituir a validação local do EXE.

Fluxo obrigatório:

```text
pacote recebido
   ↓
extração controlada
   ↓
parsing
   ↓
validação local
   ↓
[ válido ] → biblioteca/local jogável
[ inválido ] → NaoValidados / rejeitado
```

O formato importável permanece declarativo: sem scripts, includes, paths arbitrários ou execução de código.

## Regra de progressão

Nenhuma fase seguinte começa enquanto a anterior não tiver:

- implementação coerente;
- testes relevantes;
- documentação atualizada;
- branch própria;
- PR aberta;
- PR integrada em `main`.

Antes de cada novo passo: integrar o PR anterior, abandonar a branch anterior e criar uma branch nova a partir do `main` atualizado.

A mesma regra aplica-se ao Gate 9.4.5: é uma etapa própria do roadmap e deve ter branch/PR própria depois de a integração visual 9.4 ser concluída.