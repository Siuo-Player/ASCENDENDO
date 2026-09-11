# ASCENDENDO — Modelo de progresso do produto 1.0

Este documento define como interpretar o progresso do produto sem transformar o projecto numa folha de percentagens.

## Princípio

O progresso relevante é **distância real ao produto 1.0**, medida por evidência observável. Volume de código, documentação, commits, PRs ou número de testes não são, por si só, progresso de produto.

Uma área pode estar tecnicamente madura e ainda contribuir pouco para o resultado final se o utilizador continuar sem conseguir completar um fluxo importante.

## Ordem

```text
Fundação
  ↓
Gameplay
  ↓
Conteúdo
  ↓
Validação
  ↓
Produto
  ↓
Release
```

A existência de trabalho paralelo não altera esta dependência. Pode haver trabalho simultâneo em várias áreas, mas uma área só deve ser considerada concluída quando houver evidência do comportamento suportado no produto.

## Métricas que interessam

Não há percentagens oficiais fixas para as áreas do roadmap. As antigas estimativas percentuais foram retiradas porque davam uma aparência de precisão superior à evidência disponível e podiam esconder alterações no denominador sempre que o escopo de 1.0 mudasse.

O acompanhamento deve privilegiar:

- quantidade de níveis efectivamente jogáveis;
- minutos de jogo observados;
- resultados de playtests externos;
- problemas encontrados e corrigidos;
- fluxos de produto que já funcionam de ponta a ponta;
- qualidade e capacidade de distribuição do catálogo oficial.

Quando for necessário comunicar estado, deve-se usar a evidência concreta e a limitação correspondente, em vez de uma percentagem agregada inventada.

## Conteúdo

Um nível só contribui para conteúdo concluído quando:

1. existe no formato oficial;
2. passa a validação automática;
3. abre e funciona no runtime;
4. é jogado manualmente;
5. os problemas encontrados nesse playtest são tratados.

O número de níveis planeado é uma hipótese de produto sujeita a revisão pelos resultados de playtesting e pela capacidade de manter qualidade. Não deve ser usado como denominador fixo para fabricar uma percentagem de progresso.

## Critério final

ASCENDENDO chega a 100% apenas quando um utilizador externo consegue instalar, abrir, compreender, jogar, criar/editar, testar, guardar, voltar a abrir e utilizar o conteúdo oficial distribuído sem depender de conhecimento interno do repositório ou de tooling de desenvolvimento.
