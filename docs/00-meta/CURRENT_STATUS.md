# ASCENDENDO — Estado actual

**Data de referência:** 2026-09-10  
**Fonte:** `main` + código/documentação actualizados nesta tranche.

## Produto

O núcleo jogável está funcional e a campanha é definida por `Game/Assets/Levels/campaign.txt`. O projecto já dispõe de editor de níveis integrado e editor de campanha, pelo que “construir o editor” deixou de ser o objectivo principal.

O foco passa para conteúdo e validação humana.

## Sistemas existentes

- Vulkan + GLFW e renderização por `RenderSnapshot`.
- Física fixed timestep a 60 Hz.
- Commitment Jump a 60° sem controlo aéreo.
- Câmara vertical e viewport lógico 640×360.
- Replay/save states e histórico de runs.
- Menu, pausa, créditos e bindings persistentes.
- `GameState::EDITOR` para autoria de níveis.
- `GameState::CAMPAIGN_EDITOR` para ordenação/abertura de campanha.
- Validação síncrona e assíncrona do documento do editor.
- Composição de plataformas 8-neighbour, incluindo adjacência cross-region.
- Swapchain recreation transaccional/fail-closed.
- CI Linux/Windows e deterministic capture.

## Conteúdo

A campanha base foi historicamente pequena e este é o principal défice de produto identificado. Esta tranche adiciona níveis ao conjunto activo, sem alterar as regras físicas ou criar novo tooling.

## Assets

O pipeline de aprovação/proveniência continua correcto como mecanismo de segurança, mas a integração de arte externa só deve acontecer quando existir um ficheiro binário concreto. Não se considera um candidato “integrado” apenas por metadata.

## O que fica deliberadamente fora desta tranche

- nova expansão do compositor;
- novas auditorias formais sem incidente concreto;
- nova camada de roadmap;
- optimizações sem profiling;
- mudança da mecânica central.

## Próxima verificação obrigatória

Executar:

```text
python Development/AI_Validation/ai_validator.py --campaign
```

e depois fazer uma run real da campanha. O primeiro resultado testa alcançabilidade algorítmica; o segundo testa se o jogo é realmente compreensível e jogável.
