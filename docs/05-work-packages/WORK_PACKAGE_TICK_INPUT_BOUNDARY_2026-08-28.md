# Work Package — Semantic TickInput boundary

## Identificação

**Roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`  
**Work Package:** `Semantic TickInput contract`  
**Issue:** `#138`  
**Implementation branch:** `refactor/semantic-tick-input-boundary-20260828`

## Contexto

`Game/Logic/Player.h` incluía `Game/Logic/InputManager.h` apenas para obter o tipo `TickInput`. `Player` não utiliza a API do `InputManager`; consome somente o estado semântico já convertido para um tick de simulação.

## Problema / evidência

A dependência observada era:

```text
Player
  ↓
InputManager
  ↓
hardware / GLFW
```

mas o contrato necessário pelo gameplay é somente:

```text
semantic TickInput
  ↓
Player
```

`TickInput` contém cinco booleanos de intenção de gameplay e não possui estado de hardware, janela ou bindings.

## Decisão

Extrair `TickInput` para `Game/Logic/TickInput.h`.

`InputManager` continua responsável por converter input físico/configurado em `TickInput`; `Player` depende apenas do contrato semântico.

Não mover o tipo para `Core`: a evidência atual mostra que ele é um contrato específico da simulação/gameplay, não um conceito transversal de toda a aplicação.

## Invariante

A estrutura mantém exatamente os cinco campos e a mesma ordem/semântica:

```text
left
right
jumpHeld
jumpPressed
jumpReleased
```

A construção agregada existente deve permanecer válida.

## Escopo

- criar `Game/Logic/TickInput.h`;
- remover a definição de `TickInput` de `InputManager.h`;
- mudar `Player.h` para incluir apenas `TickInput.h`;
- tornar o teste de Player dependente explicitamente do contrato semântico;
- preservar `InputManager::tickInput()` e a política de bindings;
- sincronizar documentação.

## Fora de escopo

- redesign do `InputManager`;
- alteração de key bindings;
- alteração de física ou movimento;
- alterar `TickInput` para enum/class mais complexo;
- mover input semântico para `Core` sem evidência adicional.

## Validação

```text
header dependency removed
→ aggregate construction preserved
→ existing Player characterization
→ full Linux tests/headless Vulkan
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
Player.h não inclui InputManager.h
+ TickInput canónico em Logic/TickInput.h
+ InputManager continua produtor do contrato
+ comportamento de Player preservado
+ três workflows obrigatórios verdes
+ documentação sincronizada
```

## Próxima decisão

Após o #138, repetir a auditoria de ownership apenas para acoplamentos com evidência concreta. Não transformar headers pequenos em uma refatoração global de pacotes.
