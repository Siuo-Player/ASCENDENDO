# Roadmap de desenvolvimento

## Estado de referência

`main` contém o fim da tranche determinística da 9.4 (`38406c5`).

Concluído:

- Fases 1–8: motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: rato + conversão window/logical + menus clicáveis.
- 9.3: `GameState::EDITOR`, acesso por menu/tecla, câmera livre e grelha.
- bootstrap GLFW/Vulkan robusto + cleanup de falhas parciais.
- documentação técnica inicial.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.

## Fase 9 — Editor de níveis

### 9.1 ✅ — Controlos

Concluída. Intenção lógica, rebind e persistência existem.

### 9.2 ✅ — Input de rato e viewport

Concluída. Cursor, botões, `windowToLogical()` e hit-test existem.

### 9.3 ✅ — Estado EDITOR

Concluída. O editor abre dentro do jogo, com câmera livre e grelha.

### 9.4 ▶ — Manipulação visual de entidades

Tranche 1 concluída. Falta a integração visual/interativa:

- STAMP + tamanho médio por defeito;
- DRAG para criar dimensões;
- presets pequeno/médio/grande;
- seleção/movimento;
- apagar por rato e teclado;
- indicador visual de ferramenta/modo/seleção;
- renderização do documento editável dentro de `GameState::EDITOR`;
- todas as operações espaciais continuam sujeitas ao snap e bounds já testados.

### 9.5 — Guardar + validar

- serializar o documento para `.lvl`;
- preservar níveis em construção em `NaoValidados/`;
- executar validação em background;
- permitir sair enquanto a validação corre;
- notificar resultado;
- **qualquer mapa importado ou descarregado da Internet tem de ser novamente validado pelo próprio EXE antes de poder ser jogado**;
- um mapa só é marcado como jogável depois de a validação local passar.

### 9.6 — Editor de campanha

- lista de níveis em estilo playlist;
- reordenar a campanha;
- abrir um nível diretamente no `GameState::EDITOR` através de uma transição de estado;
- persistir apenas `campaign.txt`;
- não mover automaticamente ficheiros entre `Levels`, `Unused` e `NaoValidados`.

## Fase 11 — Partilha e biblioteca de mapas

A conectividade deixa de ser uma ideia abstrata e passa a fazer parte do produto, mas como camada opcional: **o jogo base continua totalmente funcional offline**.

### 11.1 — Export/import local

- botão **Exportar/Partilhar**;
- botão **Importar mapa**;
- pacote compacto para distribuição de mapas;
- o pacote deve conter dados suficientes para reproduzir o mapa sem depender do repositório de desenvolvimento;
- ao importar, o EXE extrai para uma área temporária/controlada, valida e só depois disponibiliza o mapa para jogar;
- mapas inválidos nunca entram diretamente na campanha jogável.

### 11.2 — Biblioteca online

Site dedicado para:

- upload de mapas/pacotes;
- download de mapas/pacotes;
- páginas de mapas e metadados;
- eventualmente contas, autores, favoritos, versões e pesquisa.

A primeira versão pode ser simplesmente HTTP(S) com upload/download. **WebSockets não são requisito inicial.**

### 11.3 — Partilha direta entre computadores

Possibilidade futura, separada do site:

- envio direto de um pacote entre dois computadores;
- pode usar um canal temporário/servidor de rendezvous ou outro mecanismo simples;
- só introduzir WebSockets/WebRTC/etc. quando existir uma necessidade concreta de comunicação bidirecional em tempo real.

## Fase 12 — Release / Portable Build

Objetivo final do projeto:

> um executável Windows x64 que possa ser copiado para outro computador suportado e executado sem instalar o ambiente de desenvolvimento.

Requisitos:

- `.exe` standalone/portable;
- assets e DLLs necessárias na própria pasta/distribuição;
- sem consola visível na versão final;
- sem paths absolutos;
- sem escrever fora da pasta do jogo, salvo decisão explícita futura;
- sem downloads obrigatórios para iniciar o jogo;
- deteção amigável de requisitos ausentes;
- tentar funcionar no hardware mais fraco que suporte corretamente o jogo/Vulkan;
- usar Vulkan como requisito gráfico real, não prometer suporte a máquinas sem Vulkan.

“Qualquer computador” significa, na prática, **qualquer Windows x64 dentro dos requisitos mínimos definidos pelo projeto**, e não qualquer PC existente.

## Regra de segurança de mapas

A autoridade final de jogabilidade é o executável do próprio jogo.

Nem:

- `reorganize.py`;
- o servidor do site;
- nem um campo `validated=true` enviado por outro computador

pode substituir a validação local do EXE.

Um mapa importado/descarregado segue sempre:

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

Isto permite que o servidor seja apenas uma fonte de distribuição e não uma autoridade de segurança/correcção.

## Regra de progressão

Nenhuma fase seguinte começa enquanto a anterior não tiver:

- implementação coerente;
- testes relevantes;
- documentação atualizada;
- branch própria;
- PR aberta;
- PR integrada em `main`.

Antes de cada novo passo: integrar o PR anterior, abandonar a branch anterior e criar uma branch nova a partir do `main` atualizado.