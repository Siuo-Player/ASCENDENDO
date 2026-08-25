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

## Meta de runtime

A meta de desempenho passa a ser:

```text
60 FPS consistentes como piso
+
120 FPS quando o hardware conseguir manter essa taxa de forma consistente
```

A avaliação deve usar frame-time e custo da simulação, não apenas FPS médio. A taxa física pode permanecer independente da taxa de apresentação.

## Próximos blocos P0

1. Fechar o contrato do Level Editor de uma única tela `640x360`.
2. Tornar acquire/fence/submit/present robustos a retornos antecipados.
3. Implementar recreação segura da swapchain para `OUT_OF_DATE`/`SUBOPTIMAL`.
4. Verificar graphics/present support separadamente.

## Critério da branch

Não iniciar novas funcionalidades significativas enquanto os P0 desta auditoria não estiverem fechados e testados.

## Evidência

A política temporal baseia-se no problema conhecido de catch-up ilimitado/spiral-of-death em fixed timestep. O limite numérico escolhido é uma decisão de engenharia e deve ser validado por profiling.

Os princípios de modularidade/testabilidade são avaliados por coupling, cohesion, complexidade e responsabilidades, apoiados por estudos empíricos; nomes de classes não são tratados como evidência científica.

As regras de swapchain/queues/fences serão implementadas segundo o contrato oficial do Vulkan.
