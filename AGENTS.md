# AGENTS.md — como qualquer IA deve trabalhar neste repositório

Este ficheiro é o ponto de entrada para **qualquer agente de IA** que trabalhe no ASCENDENDO — Claude, Gemini, ChatGPT, ou outro. Não é específico de um fornecedor. Lê isto antes de tocar em código ou documentação, e antes de assumir o que "já foi corrigido" — várias vezes neste projeto o que os documentos afirmam e o que o `main` realmente contém já divergiram no espaço de horas.

Se este ficheiro alguma vez contradisser `docs/ROADMAP.md`, o `ROADMAP.md` é que está certo — mas se isso acontecer, corrige este ficheiro também, na mesma PR.

## 1. Ordem de leitura antes de qualquer alteração

1. **Este ficheiro**, inteiro.
2. `docs/ROADMAP.md` — prioridade actual do produto. É o único documento vivo de planeamento.
3. `docs/PRODUCT_VISION.md` e `docs/CAMPAIGN_EDITOR_DESIGN.md` — restrições de design que não são negociáveis por conveniência de implementação.
4. `docs/ARCHITECTURE.md` — forma actual do sistema.
5. `docs/TECH_DEBT.md` — o que já se sabe estar por resolver. Não redescobrir o que já lá está.
6. `docs/CONTRIBUTING.md`, `docs/CI.md`, `docs/CODE_SIZE.md`, `docs/TESTING.md` — processo, verificação de tamanho de ficheiros, e como correr testes.
7. `docs/DECISIONS/*.md` relevantes para a área que vais tocar (não é preciso ler todos, só os que tocam a tua área).

Não é preciso perguntar ao utilizador onde está o estado actual — está sempre em `docs/ROADMAP.md`. Se esse ficheiro parecer desactualizado face ao código real, confia no código e assinala a divergência (ver secção 4), não assumas que o código está errado.

## 2. Regras não-negociáveis (nascidas de incidentes reais, não de preferência de estilo)

Cada regra abaixo existe porque já correu mal exactamente assim uma vez. Não são teóricas.

- **Actualiza `Development/dev_log.txt` no fim de cada sessão de trabalho, mesmo que não submetas mais nada.** Isto já falhou: uma sessão inteira que consolidou ~130 ficheiros de documentação e expandiu a campanha de 3 para 25 níveis não deixou uma única linha no dev log. O `git log` sozinho não substitui isto — não explica *porquê*, só *o quê*. Quando `dev_log.txt` atingir o limite adoptado, continua em `Development/dev_log2.txt`, depois `dev_log3.txt`, etc., mantendo cada ficheiro abaixo de 500 linhas e indicando a continuação no cabeçalho.
- **Quando o número de níveis/campanhas muda, actualiza no mesmo commit:** `Game/Assets/Levels/campaign.txt`, a secção "Estado actual" do `README.md`, e qualquer contagem em `docs/ROADMAP.md` ou `docs/PROGRESS_MODEL_1_0.md` que dependa disso. Isto também já falhou: o `README.md` chegou a dizer "15 níveis" enquanto o `campaign.txt` já tinha 25, porque a expansão para 25 aconteceu depois de o README ter sido escrito na mesma sessão. Se mexeste no número, corre `grep -rn "níveis\|levels" README.md docs/ROADMAP.md docs/PROGRESS_MODEL_1_0.md` antes de terminar e confirma que todos concordam.
- **Nunca commits um ficheiro placeholder/probe directamente em `main`**, nem para testar algo trivial. Já aconteceu mais do que uma vez (`chore: placeholder` seguido de revert segundos depois, directo em `main`). Usa uma branch, mesmo para uma alteração de uma linha.
- **Não expandas `Game/Graphics/PlatformCompositor.*`** (nem lhe adiciones conceitos novos, como geometria contínua ou assinaturas de adjacência) antes de existir pelo menos um tile de terreno real, licenciado e com `content_sha256` preenchido em `Game/Assets/Sprites/PLATFORM_ASSET_REGISTRY.md`. Isto é o próprio `docs/ROADMAP.md` a dizer "não expandir o compositor por antecipação" — mas o compositor já cresceu sem conteúdo real por trás uma vez, e é fácil voltar a fazê-lo através de uma "decisão de arquitectura" em vez de código, o que tem exactamente o mesmo efeito.
- **Não cries novos ficheiros `STATUS_*.md`, `CURRENT_STATUS_*.md` ou `ROADMAP_*.md` com data no nome.** Edita `docs/ROADMAP.md` no próprio sítio. O histórico já foi todo um `docs/00-meta/` cheio destes ficheiros — foi apagado numa consolidação porque nenhum deles era a fonte de verdade sozinho, só a soma de todos.
- **Não abras um documento de incidente formal (post-mortem, com hipóteses rejeitadas e regra permanente) para uma falha de CI que desaparece ao repetir o job sem alteração de código.** Uma linha no `dev_log.txt` com o número do run e "resolvido ao repetir, causa não identificada" é suficiente e correcto — já se escreveu um documento de várias páginas para isto uma vez.
- **Verifica o script real antes de confiar na prosa da política.** Neste momento, `docs/CI.md` descreve a política de `docs/CODE_SIZE.md` como baseada em linhas (`<300`/`300–399`/`>=400`), mas `docs/CODE_SIZE.md` ainda define o gate em KiB físicos (`<40`/`40–47.99`/`>=48`), e é isso que `Development/Tools/check_source_sizes.py` aplica. Os dois documentos já não concordam entre si. Quando isto for resolvido, actualiza esta nota; até lá, assume o comportamento do script, não a prosa mais recente.
- **GLFW no Windows**: `external/glfw/` só tem headers no repositório; a lib compilada (`lib-vc2022/glfw3.lib`) não está commitada e não há instruções completas para a reconstruir fora de `.github/workflows/windows.yml`. Se precisares de compilar localmente no Windows, replica exactamente os passos desse workflow (clone do commit GLFW fixado, CMake com `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`) em vez de descarregar um binário genérico do glfw.org — um runtime MSVC diferente do resto do projecto falha a compilar de forma confusa.

## 3. Antes de escrever um documento novo, decide o peso certo

Não existe já um "portão de proporcionalidade" formal neste repositório (existiu em `docs/DEVELOPMENT_PROTOCOL.md`, que foi removido por inteiro numa consolidação de documentação e ainda não foi substituído). Até existir, aplica isto por bom senso:

```text
Trivial, local, reversível num commit (typo, formatação, rerun de CI sem alterar código)
    → uma linha no dev_log.txt ou no volume dev_logN.txt corrente. Nada mais.

Local, dentro de um subsistema, sem mudar interface pública
    → branch → PR normal. A descrição da PR chega; não precisas de um ficheiro novo em docs/.

Atravessa fronteiras de arquitectura, muda interface pública, adiciona dependência,
muda política de CI/release
    → aqui sim vale a pena um registo em docs/DECISIONS/AAAA-MM-DD-titulo.md,
      curto, uma decisão por ficheiro.

```

Na dúvida, escolhe o nível mais baixo. Custa menos escalar depois do que gerar um documento formal para uma correcção de uma linha.

## 4. "Está feito" — checklist antes de terminares a sessão

- [ ] Testes relevantes correm localmente (ou em CI) e passam.
- [ ] `Development/dev_log.txt` ou o volume `Development/dev_logN.txt` corrente tem uma entrada desta sessão.
- [ ] Se mexeste em conteúdo de campanha/níveis: `campaign.txt`, `README.md` e `docs/ROADMAP.md`/`docs/PROGRESS_MODEL_1_0.md` concordam no número.
- [ ] Não ficaram ficheiros placeholder ou commits de teste directos em `main` fora de uma PR integrada.
- [ ] Se um assistente de IA novo participou nesta sessão, `docs/AI_CREDITS.md` continua a listá-lo correctamente.
- [ ] Se criaste um documento novo em `docs/`, confirma que não estás a recriar um `STATUS_*`/`ROADMAP_*` datado — devia ser uma edição a um ficheiro que já existe.

## 5. Arquivo privado do projecto (repositório separado)

Este repositório (`ASCENDENDO`) é público. O arquivo interno — auditorias externas, retrospectivas, análises de padrões de desenvolvimento, decisões difíceis de resumir numa frase — vive num repositório **separado e privado**, propriedade da mesma conta:

```
https://github.com/Siuo-Player/Siuo-Player-PROJECT-STUDIES/tree/main/ASCENDENDO

```

Esse repositório cobre potencialmente mais do que este projecto (o nome não é específico ao ASCENDENDO); a subpasta `ASCENDENDO/` lá dentro é a que interessa aqui. Um agente sem acesso a esse repositório (por não ter as credenciais do dono) não o consegue ler nem escrever — nesse caso, produz o conteúdo como ficheiro solto e pede ao humano para o colocar lá, em vez de tentar adivinhar a estrutura interna.

Consulta esse arquivo quando fores fazer uma revisão grande, uma retrospectiva, ou quando precisares de perceber *porque* uma regra da secção 2 existe em detalhe. Não é preciso lê-lo para uma tarefa pequena e local, e não faz parte da árvore deste repositório — não criar uma pasta `project-studies/` ou equivalente aqui dentro a duplicar isso.

## 6. Sobre este ficheiro

Este documento segue a mesma regra que pede a tudo o resto: se começar a aproximar-se do tamanho do que substituiu, isso é sinal para cortar, não para continuar a acrescentar. Se uma regra da secção 2 deixar de causar problemas reais durante vários ciclos de trabalho, pode ser resumida ou removida — mas só depois de confirmar em `project-studies/` que o incidente que a gerou foi mesmo resolvido, não apenas esquecido.
