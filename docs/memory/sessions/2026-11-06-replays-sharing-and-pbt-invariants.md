# Sessão: 2026-11-06 — Replays Autenticados, Partilha Comunitária e Testes de Invariantes (PBT)

## Tarefas Concluídas
1. **Auditoria e Confirmação de Rota**:
   - Comparação exata com os contratos canónicos (`AGENTS.md`, `ROADMAP.md`, `PRODUCT_VISION.md`, `ARCHITECTURE.md`, `LEVEL_FORMAT.md`).
   - Confirmado que o projeto não sofreu "burrificação" e mantém-se 100% fiel aos princípios fundamentais:
     - Salto de compromisso a 60° com carga linear e sem controlo no ar.
     - Subida vertical contínua sem mortes ou hazards artificiais.
     - Formato de nível estrito (`NAME`, `SCREENS`, `PLATFORM x y w h`), spawn implícito derivado (`X=320, Y=16`), bandeira final derivada no topo do último nível da campanha.
     - Viewport lógico 640×360 com adaptação e fullscreen F11 preservando proporção 16:9.
     - 10 campanhas oficiais e 610 níveis consistentes.

2. **Avaliação Lean 4 vs. Testes Pragmáticos**:
   - Conclusão: Lean 4 prova propriedades num modelo formal abstrato, mas tem um fosso semântico em relação aos binários C++20 compilados pelo Clang com otimizações (`-O3`) e Vulkan.
   - Implementação de alternativas pragmáticas de alta eficácia:
     - Property-Based Testing (PBT) em `Tests/Unit/test_physics_properties.cpp` (69.000+ asserções em 4 invariantes físicos).
     - Determinismo bit-a-bit provado sob sequências idênticas de inputs.

3. **Sub-sistema de Replays & Fantasmas Autenticados (`RunReplay`)**:
   - `Game/Logic/RunReplay.h` e `Game/Logic/RunReplay.cpp`:
     - Serialização compacta de ticks (1 byte por frame a 60 Hz).
     - Envelope criptográfico HMAC-SHA256 para prevenção de adulteração e fraudes de speedrun.
     - Motor headless `verifyAgainstCampaign` para re-simulação e validação matemática de conclusão de campanha sem necessidade de GPU.
   - Integrado no loop principal de `GameSession`:
     - Gravação contínua de ticks durante o estado `PLAYING`.
     - Finalização e gravação automática de `latest_<campaignID>.replay` no cofre do jogador em `userdata/Accounts/<hash>/Replays/` na conclusão da campanha.
     - Exportação via `exportLastReplay(path)`.

4. **Ferramentas CLI de Verificação e Partilha**:
   - `Development/Tools/verify_replay.py`: CLI para inspeção e verificação criptográfica de replays.
   - `Development/Tools/package_campaign.py`: CLI para empacotar campanhas em pacotes `.ascendendo-campaign` com hashes SHA-256 e validação de alcançabilidade, além de desempacotador com imunidade a Zip Slip.
   - Testes automatizados em `Tests/Tooling/test_replay_and_sharing_tools.py` (4 testes passando 100%).

5. **Portal Web (`site/`)**:
   - Adicionada secção "Time-Attack & Comunidade" ao `site/index.html` e `site/style.css` com destaque para speedruns verificados por HMAC-SHA256 e partilha comunitária de campanhas.

## Testes Executados
- `make tests-fast`: 414 casos de teste passados, 0 falhas, 69.034 asserções.
- `make tests`: 447 casos de teste passados, 0 falhas, 69.226 asserções.
- `py -m pytest Tests/`: 53 testes passados em 12.17s.
- `python Development/Tools/validate_repository_state.py`: OK (610 níveis e catálogo consistentes).
- `python Development/Tools/validate_platform_asset_registry.py`: OK.
- `python Development/Tools/check_source_sizes.py`: OK.

## Pendências e Próximos Passos
- Submeter branch para PR / integração em `main`.
