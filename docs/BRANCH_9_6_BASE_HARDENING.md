# Branch 9.6 — Base Hardening

## Objetivo

Fechar problemas funcionais e de robustez já identificados antes de continuar a construir UX avançado, Campaign Editor ou novo conteúdo.

## Documentação que rege esta branch

- `docs/ROADMAP.md`
- `docs/BASE_ARCHITECTURE_AUDIT.md`
- `docs/ARCHITECTURE.md`
- `docs/TECHNICAL_REFERENCES.md`
- `docs/SCIENTIFIC_REFERENCES.md`
- `docs/RESEARCH_INDEX.md`
- `docs/PRODUCT_DECISIONS.md`

## Concluído neste bloco

### Input

- Gameplay usa o mesmo mapeamento configurável de ações usado pelo restante da aplicação.
- O player deixou de conhecer teclas físicas diretamente.
- Foi acrescentado teste que altera uma binding de movimento e verifica o comportamento real do gameplay.

### Tempo de simulação

- O passo físico mantém-se em 60 Hz nesta tranche para evitar alterações não medidas à física existente.
- `advance()` rejeita tempos inválidos (`NaN`, infinito e valores não positivos).
- Catch-up após frames muito longos tem orçamento máximo de passos.
- O excesso de dívida temporal é descartado em vez de permitir backlog ilimitado.
- Foram adicionados testes para frame longo e tempos inválidos.

### Level Editor single-screen

**Estado: COMPLETED**

O Level Editor ficou definido como uma única tela lógica `640x360`, sem pan/câmera própria. Input de cursor, interação e render usam o mesmo espaço lógico; navegação espacial fica reservada ao futuro Campaign Editor.

Evidência: contrato documentado em `docs/9.6_EDITOR_VIEWPORT.md` e validado pelos testes do editor.

### Deterministic capture E2E

**PR #186 — COMPLETED**  
**Merge:** `3121a0cff83f2bc22781613164308d447ff8900b`  
**Estado:** **COMPLETED**

A cadeia de captura determinística foi validada end-to-end para os níveis ativos `0`, `1` e `2`, incluindo staging dos assets no layout esperado pelo runtime, geração PPM, validação estrutural e retenção dos três artefactos de CI.

A execução headless pede `1280x720` ao Xvfb, mas o `Window::create()` aplica a política existente de limitar a janela a 90% do monitor. No runner `1280x720`, o framebuffer efetivo é `1152x648`; o validator passou a validar essa dimensão real em vez de mascarar o comportamento do runtime.

Esta tranche não prova qualidade visual, golden-image equivalence, gameplay correctness ou frame-rate independence.

## Meta de runtime

A meta de desempenho passa a ser:

```text
60 FPS consistentes como piso
+
120 FPS quando o hardware conseguir manter essa taxa de forma consistente
```

A avaliação deve usar frame-time e custo da simulação, não apenas FPS médio. A taxa física pode permanecer independente da taxa de apresentação.

## Próximos blocos P0

1. Caracterizar e endurecer explicitamente o contrato de `acquire/fence/submit/present` para retornos antecipados e estados de erro.
2. Verificar a recriação da swapchain como transição segura de recursos dependentes da swapchain.
3. Consolidar a separação entre graphics/present support e os testes da capability matrix já introduzidos.

## Critério da branch

Não iniciar novas funcionalidades significativas enquanto os P0 de lifecycle Vulkan ainda tiverem propriedades operacionais não demonstradas.

## Evidência

A política temporal baseia-se no problema conhecido de catch-up ilimitado/spiral-of-death em fixed timestep. O limite numérico escolhido é uma decisão de engenharia e deve ser validado por profiling.

Os princípios de modularidade/testabilidade são avaliados por coupling, cohesion, complexidade e responsabilidades, apoiados por estudos empíricos; nomes de classes não são tratados como evidência científica.

As regras de swapchain/queues/fences são tratadas segundo o contrato oficial do Vulkan e, quando possível, caracterizadas por testes determinísticos antes de alterar produção.
