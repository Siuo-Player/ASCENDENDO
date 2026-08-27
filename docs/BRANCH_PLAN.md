# Plano da branch atual

**Bloco do roadmap:** `9.6 Base Engineering Gate`

**Work Package:** `9.6 Gate Evidence Sync`

**Branch:** `docs/9-6-gate-evidence-sync`

## Base confirmada

`main` integra PR #70, que adicionou cobertura Linux/Clang com ASan + UBSan sobre os objetos de testes e sobre a biblioteca `Game` ligada aos testes.

## Descoberta desta tranche

A documentação normativa estava inconsistente: `docs/CODE_SIZE.md` e `Development/Tools/check_source_sizes.py` usam uma política física em KiB, enquanto `DEVELOPMENT_PROTOCOL.md` ainda descrevia uma política histórica baseada em linhas. Além disso, o protocolo ainda descrevia PR #20 como ativo apesar de a documentação de arquitetura já o tratar como encerrado/superseded.

## Decisão

Sincronizar os documentos de governação com o estado real de `main`, sem alterar thresholds de source-size nem comportamento do runtime.

A política atual é:

```text
< 40 KiB       normal
40–47.99 KiB   warning
>= 48 KiB      error
```

LOC permanece apenas como diagnóstico.

## Em escopo

- atualizar `DEVELOPMENT_PROTOCOL.md`;
- atualizar `ROADMAP.md` com a evidência de sanitizer;
- atualizar `TECH_DEBT.md` com a conclusão da dívida Linux ASan/UBSan;
- manter `BRANCH_PLAN.md` e o work package sincronizados;
- preservar explicitamente as dívidas ainda abertas.

## Fora de escopo

- Windows CI;
- Vulkan lifecycle/queue hardening;
- `main.cpp` decomposition;
- RenderSnapshot geral;
- alterações de código/runtime.

## Validação

```text
PR #70 merged
→ workflow Tests verde no commit de implementação
→ workflow Sanitizers verde no commit de implementação
→ revisão de consistência documental
→ PR desta tranche
```

## Próxima tranche

Após integrar esta sincronização, abrir uma nova branch a partir do `main` atualizado para a decomposição arquitetural de `main.cpp`, começando por uma investigação do ownership graph e dos consumidores reais antes de qualquer extração.
