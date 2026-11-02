# Procedimento — Compilação e Verificação Completa do ASCENDENDO

Este procedimento deve ser executado antes de submeter alterações ou abrir Pull Requests.

## 1. Testes Rápidos (TDD)
```powershell
make tests-fast
```
Compilação incremental e execução da suite rápida (exclui passes pesados de renderização).

## 2. Suite de Testes Completa C++ (436 casos)
```powershell
make tests
```
Executa a totalidade dos testes unitários, de integração e de aceitação determinística.

## 3. Compilação do Executável do Jogo
```powershell
make game
```
Gera `build/game/game.exe` em modo Release otimizado.

## 4. Testes Python (Tooling e Física)
```powershell
python -m pytest Tests/
```
Executa 46 testes cobrindo validador de IA, física de simulação, integridade do catálogo e assets.

## 5. Gate de Consistência do Repositório
```powershell
python Development/Tools/validate_repository_state.py
```
Garante que a contagem canónica de 610 níveis está perfeitamente alinhada entre `catalogue.txt`, `README.md` e `docs/ROADMAP.md`.

## 6. Validação Mecânica do Catálogo
```powershell
python Development/AI_Validation/validate_campaign.py --catalogue
```
Valida a reachability física e as rotas dos 610 níveis das 10 campanhas.
