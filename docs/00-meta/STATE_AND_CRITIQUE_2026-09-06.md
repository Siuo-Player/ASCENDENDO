# ASCENDENDO — Estado, crítica e correção de rumo

**Snapshot:** 2026-09-06  
**Main observado:** `4cde0acc594c5f26f82d001cba5203906a3b53e3`

## Fotografia

O editor, persistência, campanha e cadeia semântica de apresentação estão tecnicamente integrados. A gestão de campanha já valida referências e persiste `campaign.txt`; o compositor foi exercitado sobre corpus real; assets possuem gate de aprovação humana separado da elegibilidade técnica.

## Críticas e correções

### 1. Progresso documental antecedeu por vezes o artefacto final
Foram necessários vários passos para transformar “candidato/preview/licença” em evidência concreta. Isto demonstra que metadados não devem ser confundidos com o ficheiro real.

**Correção:** um asset só entra no runtime depois de existir o binário exato, provenance, dimensões verificadas, validação técnica e decisão humana explícita.

### 2. Validação do editor não é ainda sinónimo de validação física/campaign completa
A `EditorValidationTask` usa snapshot imutável e o validador semântico C++ existente. A ponte direta para `Development/AI_Validation/ai_validator.py` permanece uma decisão separada.

**Correção:** manter as camadas separadas e apenas integrar o validador físico completo quando existir requisito de produto; nunca esconder a diferença no estado do editor.

### 3. Apresentação estrutural não equivale a qualidade visual
16×16, topologia, RenderSnapshot e deterministic capture demonstram estabilidade técnica, não legibilidade nem qualidade artística.

**Correção:** manter `technical`, `capture` e `human visual validation` como gates diferentes.

### 4. 9.9 não deve ultrapassar a base visual
A cena de props não deve mascarar a ausência de assets concretos ou avaliação visual humana.

**Correção:** exact binary → integration → deterministic capture → human visual validation → curated props.

### 5. Audio não deve ficar acoplado a regras

**Correção:** tratar audio como workstream próprio, usando eventos semânticos e sem duplicar lógica de gameplay.

## Gates corrigidos

```text
editor correctness
→ persistence correctness
→ campaign reference correctness
→ presentation structural correctness
→ exact assets + provenance
→ technical asset integration
→ deterministic capture
→ human visual validation
→ props/environment
→ audio
→ release hardening
```

## Antes do próximo marco

1. integrar apenas ficheiros binários concretamente disponíveis;
2. verificar provenance, dimensões e compatibilidade;
3. executar CI e deterministic capture da integração;
4. não promover assets apenas com base em preview ou nome de pacote;
5. realizar revisão visual humana antes de declarar 9A completo;
6. manter o validador físico separado até existir necessidade concreta.

## Regra permanente

```text
metadado ≠ ficheiro
CC0 ≠ aprovado
CI ≠ qualidade visual
capture ≠ playtest
validação semântica ≠ validação física completa
```
