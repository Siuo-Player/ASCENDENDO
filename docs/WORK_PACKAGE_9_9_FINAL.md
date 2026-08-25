# Work Package 9.9-final — remoção do RendererFacadeAdapter

## Resultado
Concluído.

- `main.cpp` usa `RendererFacade` diretamente.
- `RendererFacadeAdapter.cpp` já não existe.
- `RendererFacadeAdapter.h` foi removido nesta tranche.
- `RendererFacade` possui o snapshot do editor e converte `GameState` na fronteira de presentation.
- Roadmap e dívida arquitetural foram atualizados.

## Critério de saída
Apenas `RendererFacade` é usado pelo runtime/testes; não existe código de compatibilidade para o adapter.
