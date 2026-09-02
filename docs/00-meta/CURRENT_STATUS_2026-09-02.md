# ASCENDENDO — Snapshot operacional — 02/09/2026

## Baseline exato

- `main`: `dea52b5f78698b1636da349880a31b5675a0daa8`
- O snapshot cobre o estado observado até 02/09/2026.
- Não existem PRs abertos no ASCENDENDO neste momento.

## O que mudou desde o último snapshot

Desde o estado de 26/08, o projeto avançou do hardening de base e editor para uma arquitetura de apresentação de plataformas significativamente mais formalizada.

PRs recentes integradas:

| PR | Estado atual | Resultado |
|---|---|---|
| #198 | merged | piloto estrutural de compositor 16×16 |
| #199 | merged | lattice local 16×16 com origem mundial contínua |
| #200 | merged | seleção determinística de assets |
| #201 | merged | registry de proveniência dos candidatos A–D |
| #203 | merged | campos de proveniência completos |
| #204 | merged | corpus estrutural F01–F12 refeito na main atual |
| #205 | merged | catálogo de metadata visual do player |
| #207 | merged | execução sobre corpus real de `Inicio` |
| #208 | merged | fixtures + ranking determinístico |
| #209 | merged | adjacência entre regiões contínuas |
| #210 | merged | invalidação local limitada |
| #211 | merged | fundação de metadata de assets |
| #213 | merged | assinatura semântica de 8 vizinhos |
| #214 | merged | contactos cross-region na assinatura |
| #215 | merged | ponte `LevelData → apresentação` |
| #216 | merged | integração compositor → `RenderSnapshot`/`WorldRenderer` |
| #217 | merged | validação do snapshot em campanhas reais |
| #218 | merged | request canónico célula → asset |
| #219 | merged | requests de asset inválidos falham fechados |

Os PRs #198–#219 devem ser lidos como uma sequência de fundação/apresentação, não como prova de que a arte final do jogo já está aprovada.

## Estado técnico atual

### Editor

`GameState::EDITOR` já está integrado no próprio motor. Há câmara livre, grelha visual independente do snap de autoria, acesso por tecla/menu e infraestrutura de input de rato/viewport.

As sub-fases de manipulação de entidades, persistência/validação assíncrona e gestão de campanha continuam por implementar.

### Compositor de plataformas

A fundação estrutural 16×16 está implementada e ligada ao caminho de apresentação:

```text
LevelData / runtime Level
        ↓
PlatformPresentationRasterizer
        ↓
semantic RegionCell
        ↓
8-neighbour signatures + cross-region contacts
        ↓
RenderSnapshot
        ↓
WorldRenderer
        ↓
asset request / deterministic selector
```

Os contratos de apresentação preservam coordenadas mundiais contínuas e não reescrevem a geometria do gameplay.

### Assets

Existe uma camada de metadata/candidatos e seleção determinística, incluindo fail-closed para pedidos inválidos. Existem referências CC0 pesquisadas e provenance registada.

Ainda não existe base suficiente para afirmar que uma determinada família visual é a arte final correta para o produto. A promoção de assets deve continuar a exigir verificação de source/licence/provenance, compatibilidade técnica, captura determinística e revisão visual explícita.

### Evidência visual

A infraestrutura de deterministic capture já foi ampliada para 16:9, 4:3 e 21:9, incluindo níveis canónicos. Isto prova propriedades técnicas da execução gráfica nesses cenários; não substitui revisão visual humana.

O protocolo de human playtest existe, mas este snapshot não encontrou evidência de resultados humanos concluídos que justifique declarar o design visual validado.

### CI

A topologia atual é deliberadamente pequena:

```text
.github/workflows/tests.yml
.github/workflows/windows.yml
.github/workflows/deterministic-capture.yml
```

A antiga smoke workflow foi removida. Não deve voltar a ser criada apenas como “teste de que Actions funciona”.

## O que estava desatualizado/errado na roadmap anterior

### 1. O compositor ainda aparecia como futuro

Já não é correto dizer que a apresentação semântica 16×16, os 8 vizinhos, cross-region contacts, ponte com `LevelData`, integração com `RenderSnapshot` e asset-request mapping estão simplesmente “por fazer”. Essas camadas já estão integradas.

### 2. A Fase 9 estava demasiado centrada no editor

O Editor continua central, mas o estado real do projeto criou um segundo workstream legítimo: apresentação/asset pipeline. A roadmap foi separada em Editor, Presentation, Evidence e Audio.

### 3. “Testes confirmados 135/135” estava mal sincronizado

O README antigo guardava números esperados de testes ligados ao estado 9.3. Esses números não devem ser tratados como snapshot atual até uma execução real do baseline atual os confirmar.

### 4. “Versão 9.3” deixou de ser um bom identificador global

Os marcadores `v9.3` continuam úteis como histórico local de implementação do Editor, mas não descrevem as tranches de compositor/asset de 02/09. O novo roadmap datado passa a ser a referência operacional.

### 5. Referência CC0 não é aprovação de arte

Os candidatos de plataforma/props foram pesquisados e documentados, mas continuam sujeitos a uma pipeline explícita de aprovação. “Encontrado e licenciado” e “escolhido para o produto” são estados diferentes.

### 6. Evidência estrutural não é evidência estética

Fixtures, testes unitários e deterministic capture devem ser usados para provar invariantes técnicas. A pergunta “fica visualmente bom?” pertence à revisão visual/human playtest.

## Correções de método incorporadas

O ASCENDENDO deve seguir estas fronteiras de forma permanente:

```text
DOMÍNIO / GAMEPLAY
≠
PRESENTATION SEMANTICS
≠
ASSET SELECTION
≠
VISUAL APPROVAL
≠
HUMAN USABILITY EVIDENCE
```

Para cada novo subsistema:

1. definir contrato;
2. criar teste estrutural mínimo;
3. validar com corpus real quando disponível;
4. medir deterministicamente;
5. só então procurar aprovação humana ou generalização.

Entradas inválidas devem falhar fechadas quando não exista uma interpretação segura.

## Próximos bloqueios reais

O próximo trabalho útil não é importar dezenas de sprites. É completar a autoria do Editor e tornar a pipeline de apresentação observável no uso normal.

Ordem:

```text
9.4 entity manipulation
→ 9.5 save + async validation
→ 9.6 campaign management
→ broader real-campaign compositor coverage
→ explicit visual asset approval
→ small curated props scene
→ human visual validation
→ audio design/implementation
→ release hardening
```

## Áudio

O áudio passa a ser um workstream explícito da Fase 9, não um adorno de última hora. Deve consumir eventos semânticos do jogo/editor e manter a implementação de som separada das regras.

Eventos iniciais:

```text
jump charge / release
land
fail/death
checkpoint/goal
menu confirm/cancel
editor place/move/delete
validation success/failure
```

Também deve existir provenance/licence dos sons adotados.

## Relação com PROJECT-STUDIES e RedWar

A investigação transversal recente reforçou uma regra útil para ASCENDENDO: primeiro garantir contratos e reprodutibilidade, depois otimizar/generalizar.

Isto é uma transferência de metodologia, não de arquitetura. ASCENDENDO não deve incorporar Ares, NNUE, search architecture ou o `Siuo-Engine` como dependências por causa desta aprendizagem.

## Estado resumido

```text
BASE ENGINE              ✅ forte
EDITOR FOUNDATION        ✅
EDITOR AUTHORING         🟡 próximo
COMPOSITOR FOUNDATION    ✅
REAL CAMPAIGN BRIDGE     ✅
ASSET SELECTION          ✅
ASSET APPROVAL           🟡 evidência pendente
PROPS SCENE               🟡 próximo após approval
DETERMINISTIC CAPTURE    ✅ infraestrutura
HUMAN VISUAL VALIDATION  🟡 protocolo, results pending
AUDIO                   🟡 novo workstream
RELEASE                  ⏳ futuro
```

## Regra de manutenção deste snapshot

Não editar este documento retroativamente para o atualizar. Cada novo estado relevante deve criar outro snapshot datado com o SHA exato da `main` e a fronteira de evidência correspondente.
