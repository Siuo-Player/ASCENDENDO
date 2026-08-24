# Plano da branch atual

**Branch:** `docs/release-and-map-sharing`

**Base:** `main` após integração da PR #4 (`38406c5`).

## Estado herdado — concluído

- Fases 1–8.
- 9.1–9.3.
- bootstrap GLFW/Vulkan e cleanup parcial.
- documentação técnica inicial.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.

## Motivo desta branch

Nova decisão de produto: o resultado final deve ser um EXE portable para Windows x64 suportado, enquanto mapas podem ser partilhados localmente e através de uma futura biblioteca online.

## Implementado nesta branch

- `docs/ROADMAP.md` atualizado com o objetivo de release portable;
- fluxo offline-first explicitado;
- regra de que o EXE é a autoridade final para validar mapas importados/descarregados;
- `docs/MAP_SHARING.md` com import/export, site de mapas e decisão de não introduzir WebSockets sem necessidade concreta;
- `docs/RELEASE.md` com requisitos de distribuição, hardware, assets, paths, diagnóstico e testes de release.

## Decisões de produto registadas

- mapa pode ser enviado diretamente como pacote;
- editor terá `Importar` e `Exportar/Partilhar`;
- site pode hospedar upload/download e metadados;
- conectividade é opcional para gameplay;
- HTTP(S) é suficiente para a primeira biblioteca online;
- WebSockets/WebRTC/etc. só entram se existir uma funcionalidade bidirecional em tempo real que realmente os exija;
- máquinas sem Vulkan adequado não são suportadas, mas devem receber erro gracioso;
- objetivo de hardware: o mais fraco possível dentro dos requisitos Vulkan do jogo.

## Não entra nesta branch

- implementação do site;
- networking no EXE;
- implementação do pacote de mapas;
- release build;
- UI do editor.

## Próxima branch

Depois desta documentação ser integrada, abrir uma branch nova para a UI da 9.4. A primeira tarefa é renderizar o `LevelEditorDocument` dentro de `GameState::EDITOR` e ligar mouse/window-to-logical/hit-testing ao modelo. Depois entram STAMP/DRAG, seleção, mover e apagar.