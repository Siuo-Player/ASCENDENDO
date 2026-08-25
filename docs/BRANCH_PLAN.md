# Plano da branch atual

**Bloco do roadmap:** `9.6 Base Engineering Gate`

**Work Package:** `Base gate alignment — tooling, evidence and remaining modularity`

## Objetivo

Fechar contradições de base antes de iniciar `RenderSnapshot` geral ou novas funcionalidades significativas. A branch começa pelo tooling que ainda não implementava a política documental e prepara a verificação dos restantes alvos de modularidade.

## Estado inicial confirmado

O `main` atual já integrou:

- hardening do input e fixed timestep;
- contrato de viewport do Level Editor `640x360`;
- lifecycle/swapchain e seleção separada de graphics/present conforme as tranches Vulkan;
- remoção do renderer legado e do adapter de migração;
- contrato inicial de `RenderSnapshot`;
- decomposição de `FontRenderer`;
- decomposição de `SpriteRenderer`.

A investigação documental e científica, incluindo `Siuo-Player-PROJECT-STUDIES/ASCENDENDO`, mostra contudo que o Gate ainda não deve ser considerado encerrado. Permanecem, em particular:

- política de source-size anteriormente divergente do checker real;
- composição excessiva em `main.cpp`;
- ficheiros de testes grandes que merecem revisão por coesão;
- necessidade de evidência de CI além da execução Linux/headless atual;
- fronteiras de dados/runtime ainda por consolidar.

## Descoberta principal

`docs/CODE_SIZE.md` já definia uma política por linhas (`<300`, `300–399`, `>=400`), mas `Development/Tools/check_source_sizes.py` ainda media KiB e não incluía `main.cpp`.

Isto permitia que a documentação dissesse que existia um gate que o CI, na realidade, não aplicava.

## Decisão

Corrigir primeiro o mecanismo executável, sem alterar os limiares para fazer ficheiros passarem:

```text
política normativa
      ↓
checker CI
      ↓
resultado observável
      ↓
work package de cada alvo > 300 linhas
```

`main.cpp` passa a ser explicitamente avaliado pelo mesmo checker.

## Alterações desta branch

1. Migrar o checker de KiB para linhas físicas.
2. Incluir `main.cpp`.
3. Manter `Game/` e `Tests/` abrangidos pela mesma regra.
4. Atualizar `docs/CODE_SIZE.md` para declarar a implementação real.
5. Atualizar roadmap/auditoria para distinguir claramente:
   - ferramenta alinhada;
   - warning de tamanho;
   - hard limit;
   - Gate de Engenharia ainda aberto.
6. Criar work packages apenas para decomposições justificadas por responsabilidade/cohesion, nunca por contagem arbitrária.

## Fora de escopo

- `RenderSnapshot` geral;
- Campaign Editor;
- dificuldade adaptativa;
- `AssetManager` genérico;
- otimizações sem profiling;
- substituição da política de tamanho por outra métrica;
- abstração Vulkan genérica sem consumidores concretos.

## Riscos

- novo checker revelar ficheiros já acima do hard limit;
- confundir warning com defeito funcional;
- dividir testes ou `main.cpp` artificialmente;
- declarar o Gate fechado apenas porque CI está verde;
- documentação ficar novamente atrás da implementação.

## Validação

A branch só pode ser considerada pronta para PR quando houver evidência observável de:

```text
source-size checker executado
+ resultado de todos os ficheiros relevante
+ build game
+ build/tests
+ Vulkan/headless
+ campaign validation
+ documentação sincronizada
```

Para cada refatoração posterior:

```text
responsabilidade identificada
→ consumidores/ownership mapeados
→ alternativa comparada
→ implementação mínima
→ testes de regressão
→ resultado CI
→ documentação atualizada
```

## Critério de saída

```text
checker e documentação usam a mesma política
+ main.cpp é coberto
+ nenhum erro de tamanho não tratado permanece
+ warnings restantes têm work package explícito quando exigirem ação
+ resultados CI são observáveis
+ arquitetura/roadmap correspondem ao estado real
+ RenderSnapshot geral continua bloqueado até todos os gates necessários fecharem
```

## Próximo work package após esta correção

Investigar os grandes ficheiros de testes e, depois, `main.cpp`, começando sempre por coesão, dependências e testabilidade. Só após esse gate será retomada a migração geral para `RenderSnapshot`.
