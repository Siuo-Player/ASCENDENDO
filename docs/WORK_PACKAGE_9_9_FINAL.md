# Work Package 9.9-final — remoção do RendererFacadeAdapter

## Objetivo
Eliminar a última camada de compatibilidade do renderer.

## Estado de entrada
`RendererFacade` já contém a implementação, a conversão `GameState -> RenderState` e a propriedade/atualização do `EditorRenderSnapshot`. O `RendererFacadeAdapter.cpp` já foi removido.

## Tarefas
- [ ] `main.cpp` inclui `RendererFacade.h` diretamente.
- [ ] `main.cpp` instancia `RendererFacade` diretamente.
- [ ] chamadas de `attachEditorSession`/`drawFrame` continuam equivalentes.
- [ ] `RendererFacadeAdapter.h` removido.
- [ ] documentação e dívida atualizadas.
- [ ] pesquisa por referências operacionais ao adapter sem resultados.

## Dependências
- PR #17 integrado.
- `RendererFacade` é a API pública de presentation do runtime.

## Critério de saída
Apenas `RendererFacade` é usado pelo runtime/testes; não existe código de compatibilidade para o adapter.
