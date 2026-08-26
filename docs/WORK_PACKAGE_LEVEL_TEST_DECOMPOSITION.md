# Work Package — Level test decomposition

**Roadmap:** `9.6 Base Engineering Gate`

## Discovery

`Tests/Unit/test_level.cpp` está acima do limite de inspeção e mistura três responsabilidades distintas:

1. construção/estado e geometria básica do `Level`;
2. física e resolução de colisões;
3. carregamento de ficheiros `.lvl`, offsets e regressões de streaming.

O tamanho é apenas o sinal; a divisão é justificada pelas fronteiras de responsabilidade e de diagnóstico.

## Decision

Separar a cobertura em:

- `Tests/Unit/test_level_geometry.cpp`
- `Tests/Unit/test_level_collision.cpp`
- `Tests/Unit/test_level_file_loading.cpp`

O código de produção permanece inalterado.

## Additional finding

Os testes de `appendFromFile` usam ficheiros temporários com nomes fixos. A decomposição deve manter o comportamento atual, mas a dependência fica registada como dívida de isolamento para trabalho posterior: nomes únicos/RAII e execução paralela segura.

## Validation target

Preservar a cobertura funcional da suite atual. A contagem de testes/assertions deve permanecer explicável; rearranjo de ficheiros não justifica perda de cobertura.

## Exit criteria

- `test_level.cpp` removido;
- três suites com responsabilidades claras;
- código de produção inalterado;
- testes de file-loading continuam a limpar os temporários;
- CI Linux/headless verde;
- campanha validada;
- risco de temporários fixos registado para trabalho posterior.
