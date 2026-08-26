# Work Package — Level test decomposition

**Roadmap:** `9.6 Base Engineering Gate`

## Discovery

`Tests/Unit/test_level.cpp` tinha 326 linhas e misturava construção/estado, física/colisão e file loading/streaming.

A divisão é justificada por fronteiras de responsabilidade e diagnóstico, não pelo número de linhas isoladamente.

## Decision

Separar em:

- `Tests/Unit/test_level_geometry.cpp`
- `Tests/Unit/test_level_collision.cpp`
- `Tests/Unit/test_level_file_loading.cpp`

Produção permanece inalterada.

## Additional finding

Os testes de `appendFromFile` usam ficheiros temporários com nomes fixos. Isso fica registado como dívida de isolamento para uma tranche própria, com nomes únicos/RAII e execução paralela segura.

## Exit criteria

- suite monolítica removida;
- responsabilidades separadas;
- cobertura preservada;
- CI Linux/headless e campanha verdes;
- dívida de temporários registada.
