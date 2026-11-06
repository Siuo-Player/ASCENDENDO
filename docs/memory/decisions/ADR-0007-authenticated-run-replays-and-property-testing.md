# ADR-0007: Authenticated Run Replays, Headless Verification e Property-Based Testing

## Contexto
O ASCENDENDO é uma plataforma de subida vertical de precisão, speedrunning e criação comunitária de níveis. Sendo 100% determinístico a 60 Hz e sem mortes (o único fator competitivo é o tempo cronometrado até à bandeira), surgiram duas necessidades fundamentais:
1. **Verificação de Speedruns sem Cheating**: Permitir aos jogadores partilhar tempos recorde de subida contra amigos e desconhecidos online com prova matemática e criptográfica absoluta de que a corrida é autêntica e fisicamente concluída, sem necessidade de servidores na nuvem caros ou envio de vídeos pesados.
2. **Garantia de Invariantes Físicos**: Provar que o motor de física discreta preserva propriedades fundamentais (não-penetração no chão, monotonia de salto com carga, e determinismo bit-a-bit), além de permitir embalar e partilhar campanhas comunitárias com verificação prévia de alcançabilidade física.

## Decisão

### 1. Formato de Replay Compacto e Autenticado (.replay)
- Cada tick da simulação fixa (1/60s) é codificado num único byte de máscara de bits (Left, Right, JumpHeld, JumpPressed, JumpReleased), resultando em apenas ~3.6 KB por minuto de jogo.
- O ficheiro é serializado num envelope seguro com assinatura **HMAC-SHA256** (`core::Security::writeSecureFile`), tornando impossível a um jogador alterar tempos declarados no cabeçalho ou injetar inputs sem invalidar a assinatura criptográfica.

### 2. Motor de Verificação Headless (C++ & Python)
- `RunReplay::verifyAgainstCampaign(...)`: Um executor headless em C++ que re-simula a física tick-a-tick em milissegundos sem GPU, provando se a hitbox do jogador tocou a bandeira derivada no topo do último nível da campanha.
- `Development/Tools/verify_replay.py`: Ferramenta CLI para árbitros de speedrun e comunidade validarem qualquer ficheiro `.replay`.

### 3. Embalamento de Campanhas Comunitárias (.ascendendo-campaign)
- `Development/Tools/package_campaign.py`: Permite a criadores empacotar níveis criados nos editores nativos (`E` e `C`) num arquivo portátil com `manifest.json` contendo hashes SHA-256 de integridade e verificação prévia de que todos os saltos são passíveis.
- Suporte a desempacotamento seguro com prevenção ativa contra ataques Zip Slip (`unpack_campaign`).

### 4. Property-Based Testing (PBT) no Motor C++
- Em alternativa a theorem provers desacoplados da compilação nativa (como Lean), adotou-se PBT / Invariant Fuzzing em `Tests/Unit/test_physics_properties.cpp`, testando dezenas de milhares de iterações com inputs aleatórios diretamente sobre o código C++ compilado.

## Consequências
- Os jogadores têm ferramentas gratuitas para competir em speedruns de tempo com verificação anti-cheat garantida.
- Os criadores comunitários podem distribuir campanhas verificadas e protegidas contra adulteração.
- O código continua limpo, 100% nativo C++20, sem dependências externas.
