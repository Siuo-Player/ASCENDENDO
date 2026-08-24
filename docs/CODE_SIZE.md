# Regra de tamanho dos ficheiros de código

O ASCENDENDO usa uma regra de tamanho físico para evitar que componentes se tornem monólitos difíceis de manter ou modificar com segurança.

## Limites

Aplica-se apenas a ficheiros de código C/C++:

`.c`, `.cc`, `.cpp`, `.cxx`, `.h`, `.hh`, `.hpp`, `.hxx`

| Tamanho | Estado | Regra |
|---|---|---|
| `< 30 KiB` | normal | desenvolvimento normal |
| `30–36 KiB` | aviso | não adicionar novas responsabilidades; planear subdivisão |
| `> 36 KiB` | bloqueio | o ficheiro deve ser subdividido antes da integração |

Documentação, dados, assets, logs e scripts não são sujeitos a este limite.

## Porquê 36 KiB?

O objetivo não é impor ficheiros artificialmente pequenos. O limite deve deixar espaço suficiente para uma classe ou unidade de responsabilidade relativamente complexa, mas obrigar a decomposição antes de componentes centrais acumularem demasiadas responsabilidades.

A zona de aviso começa aos 30 KiB para permitir uma subdivisão planeada antes de atingir o bloqueio.

## Subdivisão

Quando um ficheiro atinge o limite, a divisão deve seguir **responsabilidades**, e não simplesmente cortar o ficheiro em partes arbitrárias.

Exemplo para o renderer:

```text
Renderer lifecycle/frame submission
Renderer world drawing
Renderer UI/HUD
Renderer editor
```

A divisão deve preservar ownership, interfaces simples e dependências unidirecionais.

## CI

`Development/Tools/check_source_sizes.py` verifica automaticamente `Game/` e `Tests/` e falha quando um ficheiro ultrapassa 36 KiB.
