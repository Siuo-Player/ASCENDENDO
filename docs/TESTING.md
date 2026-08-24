# Testes e validação

## Objetivo

O projeto usa várias camadas de testes porque nem todos os problemas de um motor Vulkan podem ser encontrados por testes unitários, mas os testes gráficos também não devem ser obrigatórios para validar cada algoritmo de gameplay.

## Camadas

### Unit

Testam lógica sem GPU sempre que possível:

- física e AABB;
- Player;
- Level;
- Camera;
- InputManager;
- GameAction;
- KeyBindings;
- Viewport;
- CampaignID;
- RunHistory;
- futura lógica do editor.

### Integration

Testam a ligação com Vulkan/GLFW:

- criação da instance/device;
- janela e surface;
- swapchain;
- render pass;
- pipelines;
- renderer;
- texto e sprites.

### Validação de níveis

`Development/AI_Validation/ai_validator.py --campaign` é a verificação física da campanha usada pelo pre-push.

O simulador em `Development/AI_Validation/sim/engine.py` existe para reproduzir o passo fixo e as colisões discretas do motor. A validação analítica não deve ser tratada como mais verdadeira do que a simulação discreta: o objetivo é que ambas concordem sobre o mesmo contrato físico.

## Build normal

```bash
make clean
make tests-verbose -j8
```

Para testar apenas a parte sem integração gráfica pesada:

```bash
make tests-fast
```

Para compilar o jogo:

```bash
make game
```

## Headers e dependências

O Makefile usa `-MMD -MP` e inclui os `.d` gerados. Isto é obrigatório porque headers como `InputManager.h` podem alterar o layout de uma classe. Sem dependências corretas, unidades de tradução antigas podem ser ligadas com unidades compiladas contra um layout diferente.

## Teste de cada nova fase

Cada subfase deve seguir:

1. escrever testes da regra nova;
2. confirmar que falham sem implementação quando a regra é testável isoladamente;
3. implementar;
4. executar unit tests;
5. executar integration tests afetados;
6. executar `make tests-verbose` completo;
7. validar a campanha;
8. documentar resultados antes de abrir a PR.

## Editor

A lógica de edição deve ser testável sem Vulkan. Por exemplo:

```text
pixel de janela
    ↓
Viewport::windowToLogical
    ↓
coordenada lógica
    ↓
snap ao grid
    ↓
validação de bounds
    ↓
mutação do LevelDocument
```

Assim, erros de coordenadas, grid e limites não ficam dependentes de testes manuais no ecrã.

## O que não deve ser feito

Não reduzir testes antigos para fazer uma feature passar. Não substituir uma regra física por uma aproximação só porque simplifica o teste. Não considerar "compila" equivalente a "está validado".