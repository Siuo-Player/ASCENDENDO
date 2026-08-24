# Partilha de mapas e biblioteca online

## Objetivo

ASCENDENDO é um jogo offline-first. O jogo completo deve funcionar sem Internet.

A conectividade existe para distribuir conteúdo criado por jogadores, não para tornar o gameplay dependente de um servidor.

## Fluxos suportados

### Exportar / Partilhar

O editor terá uma ação `Exportar/Partilhar` que cria um pacote de mapa compacto.

O pacote deve ser suficiente para:

- identificar o mapa;
- conter a definição dos níveis necessários;
- conter metadados básicos do autor/mapa;
- referenciar apenas recursos permitidos pelo formato do editor;
- ser importável noutro computador sem o repositório de desenvolvimento.

### Importar

O utilizador pode selecionar um pacote local.

O EXE deve:

1. extrair para uma localização temporária/controlada;
2. verificar o formato;
3. fazer parsing;
4. validar o mapa com a mesma autoridade usada para os mapas locais;
5. só depois o disponibilizar como mapa jogável.

Um pacote nunca pode ser considerado seguro ou jogável apenas porque contém um campo `validated=true`.

## Site de mapas

O produto pode ter um site próprio com:

- upload;
- download;
- metadados;
- autor;
- pesquisa;
- versões;
- eventualmente ratings/favoritos.

O site é uma fonte de distribuição. A decisão final sobre se um mapa pode ser jogado continua no EXE.

Fluxo:

```text
Site
  ↓ download
pacote
  ↓
EXE
  ↓ parse + validação local
válido ─────────→ jogável
inválido ───────→ rejeitado / NaoValidados
```

## WebSockets

WebSockets **não são necessários para a primeira versão**.

HTTP(S) é suficiente para upload/download e metadados.

Só devem ser introduzidos quando existir uma funcionalidade que exija comunicação bidirecional em tempo real, por exemplo:

- partilha direta em tempo real;
- colaboração simultânea no editor;
- notificações live;
- outro serviço interativo.

Se futuramente existir partilha direta entre computadores, a camada de transporte deve continuar opcional e separada do núcleo do jogo.

## Segurança e integridade

O servidor nunca substitui a validação local.

O EXE deve rejeitar:

- formato desconhecido;
- estrutura incompleta;
- entidades fora dos limites;
- níveis fisicamente inválidos;
- referências de recursos não permitidas;
- versões incompatíveis do formato.

A futura arquitetura deve considerar hashes/versionamento do pacote para detetar corrupção ou alterações após download.