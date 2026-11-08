# ADR-0014: Enriquecimento Global da Documentação e Bateria de Testes de Sistema

## Estado
Aprovado

## Contexto
1. **Consolidação Documental Multidisciplinar**: Com os avanços nas áreas de game feel (Swink 2008), desacoplamento de física e apresentação (Thorson 2018), cinemática balística analítica (Aramini 2018), segurança Zero Blast-Radius (NIST SP 800-63B) e frustum culling na pipeline Vulkan, era necessário sincronizar e enriquecer os documentos canónicos (`DESIGN_REFERENCES.md`, `SCIENTIFIC_REFERENCES.md`, `ARCHITECTURE.md`, `TESTING.md`, `LEVEL_EDITOR.md`, `PRODUCT_VISION.md`).
2. **Cobertura de Testes de Sistema**: A pasta `Tests/System/` continha apenas `.gitkeep`. Havia a necessidade de um teste de sistema holístico (`test_system_resilience.cpp`) que exercesse a interação entre múltiplos subsistemas integrados (câmara de segurança sob 10.000 ticks, cofre de contas sob corrupção de disco, cones balísticos no editor vs simulação discreta).

## Decisão
1. **Documentação Viva e Fundamentada**:
   - `docs/DESIGN_REFERENCES.md`: Adicionadas análises formais de Swink (2008), Thorson & Berry (2018), Yu (2016), Aramini et al. (2018) e Smith et al. (2008).
   - `docs/SCIENTIFIC_REFERENCES.md`: Inseridas as deduções matemáticas completas em forma fechada da cinemática a 60° e equações diferenciais de rastreio de câmara com restrições rígidas.
   - `docs/ARCHITECTURE.md`: Documentados os subsistemas `JumpTrajectory`, Viewport Frustum Culling em Vulkan, Safety Viewport Clamping e arquitetura de contas PPID.
   - `docs/TESTING.md`: Estruturada a taxonomia completa de mais de 20 categorias de testes suportadas no repositório.
   - `docs/LEVEL_EDITOR.md`: Atualizados os contratos de spawn/flag derivados, undo/redo atómico e arcos de alcance visual.
2. **Nova Bateria de Testes de Sistema (`Tests/System/test_system_resilience.cpp`)**:
   - 4 novos casos de teste de sistema validando o enquadramento de câmara ao longo de 10.000 ticks, convergência do ápice discreto vs analítico (< 1.0 px), isolamento hermético entre contas (Raio de Explosão Zero) e ciclo de vida do `GameSession`.

## Consequências
- A documentação atinge nível de qualidade e precisão de manual de engenharia e referência académica.
- O subsistema de testes passa a cobrir formalmente a categoria de Testes de Sistema (`Tests/System/`), completando a matriz de garantia de qualidade.
