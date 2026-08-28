# Work Package — LevelData semantic geometry validation

## Identificação

**Roadmap:** `Fase 10 / LevelData semantic validation`  
**Work Package:** `Semantic LevelData geometry validation`  
**Issue:** `#142`  
**Implementation branch:** `refactor/leveldata-semantic-validation-20260828`

## Contexto

`LevelDataIO` já rejeita sintaxe desconhecida, tokens inválidos, truncamento e tokens excedentes, mas a representação resultante ainda podia conter geometria semanticamente impossível para o modelo atual.

## Problema / evidência

`PLATFORM x y w h` e `FLAG x y w h` eram convertidos diretamente para `AABB` sem verificar `w > 0` e `h > 0`.

Assim, um documento com zero/negativo podia produzir uma AABB degenerada ou invertida e chegar a `Level::appendFromData()`.

## Decisão

Manter responsabilidades separadas:

```text
parse
  ↓
LevelDataIO
  ↓
validate
  ↓
LevelDataValidator
  ↓
runtime geometry
```

`LevelDataValidator` implementa apenas invariantes geométricas inequívocas do modelo atual:

```text
platform.width() > 0
platform.height() > 0
flag.width() > 0
flag.height() > 0
```

Não há ainda schema versioning, migration ou política global de bounds.

## Escopo

- adicionar `Game/Logic/LevelDataValidator.h/.cpp`;
- validar extensões positivas de plataformas e flag;
- aplicar validação em `CampaignRuntime` após parsing e antes do append;
- adicionar testes unitários e testes de integração de consumo da campanha;
- documentar `parse → validate → runtime`.

## Fora de escopo

- schema versioning;
- migration;
- mudança da política de bounds laterais/verticais;
- mudança de colisão/física;
- alteração de formato `.lvl` válido;
- redesign de `Level`.

## Invariantes

A geometria válida existente continua válida. Uma entrada sem flag continua válida. Uma entrada inválida não incrementa `currentLevelIndex`, não altera `m_spawnY` e não acrescenta plataformas ao `Level`.

## Validação

```text
validator unit tests
→ malformed campaign-runtime tests
→ Linux normal/headless Vulkan
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
syntax and semantics remain distinct
+ invalid geometry rejected before runtime append
+ current valid campaign unchanged
+ three mandatory CI gates green
+ documentation synchronized
```

## Próxima decisão

Continuar Fase 10 apenas para invariantes semânticas concretamente exigidas. Schema/versioning só deve avançar quando houver requisito de compatibilidade/importação que o justifique.
