# Release e distribuição

## Objetivo final

O resultado final do projeto deve poder ser entregue como um pacote simples, adequado a jogadores que não têm o ambiente de desenvolvimento.

### Alvo

**Windows x64**, em máquinas que cumpram os requisitos mínimos gráficos do jogo.

O jogador deverá conseguir:

1. receber uma pasta/arquivo de distribuição;
2. extrair/copiá-lo para uma localização qualquer;
3. executar o `.exe`;
4. jogar sem instalar Clang, Vulkan SDK, Python, Git, GLFW de desenvolvimento ou outras ferramentas usadas para construir o projeto.

## Requisitos do pacote

- executável principal;
- DLLs/runtime necessários;
- shaders SPIR-V;
- fontes e sprites usados pelo jogo;
- conteúdo inicial da campanha;
- ficheiros auxiliares necessários em runtime.

O pacote não deve depender da árvore do Git nem de paths absolutos.

## Hardware

Pretende-se suportar o hardware mais fraco que consiga executar corretamente a versão Vulkan do jogo.

Não é possível prometer funcionamento em computadores sem suporte Vulkan adequado. Nesses casos, o jogo deve detetar a falha durante o bootstrap e apresentar uma mensagem clara em vez de crashar.

## Offline first

A ausência de Internet nunca deve impedir:

- iniciar o jogo;
- jogar a campanha local;
- editar níveis locais;
- importar pacotes de mapas já presentes no computador;
- validar mapas.

## Mapas externos

Qualquer mapa recebido externamente passa por validação local no EXE antes de entrar no conjunto jogável.

A validação do servidor, caso exista, é apenas uma otimização/metadata. Nunca é a autoridade final.

## Consola

A versão de desenvolvimento pode utilizar terminal/logs. A versão final deve funcionar como aplicação gráfica sem abrir uma consola visível.

## Paths e persistência

Por defeito, o projeto privilegia paths relativos dentro da pasta distribuída. A política de persistência do jogador deverá ser fechada antes do Release Build e documentada claramente.

## Diagnóstico de erro

Falhas como:

- Vulkan indisponível;
- shaders ausentes;
- DLLs ausentes;
- assets essenciais corrompidos;
- mapa inválido;

devem produzir mensagens compreensíveis e encerramento limpo, não acesso inválido/crash.

## Teste de release

Antes da versão final deve existir uma máquina limpa ou ambiente equivalente onde se confirme que:

- o pacote não precisa do ambiente de desenvolvimento;
- o jogo inicia;
- a campanha funciona;
- os recursos são encontrados através de paths relativos;
- importar um mapa inválido é rejeitado;
- importar um mapa válido funciona;
- uma máquina sem Vulkan é tratada corretamente.
