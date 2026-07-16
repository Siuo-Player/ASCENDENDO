#pragma once
// =============================================================================
//  Game/Core/Viewport.h
//
//  @version 9.3
//  @history
//    v9.2 — criado. Fase 9.2: suporte a rato.
//    v9.3 — MenuBoxLayout/hitTestMenuBox generalizados para N caixas
//            (ate' entao fixo em 3). Motivo: MENU ganhou uma 4a opcao
//            ("Editor", Fase 9.3), PAUSED continua com 3 -- a largura da
//            caixa passa a ser CALCULADA a partir de `count`, com um tecto
//            de MAX_BOX_W=170 (o valor antigo, fixo). Para count=3 isto
//            reproduz EXACTAMENTE a geometria anterior (170px, verificado
//            por calculo: (640-2*18)/3=201.3 > 170 -> fica no tecto de
//            170); para count=4 encolhe automaticamente para caber
//            (640-3*18)/4=146.5px, sem alterar PAUSED em nada.
//
//  Duas responsabilidades, ambas puramente matematicas (sem Vulkan/GLFW):
//
//  1. windowToLogical(): converte uma posicao do cursor em pixeis de JANELA
//     (convencao GLFW: Y para baixo, origem topo-esquerdo) para o espaco
//     LOGICO do motor (640x360 por omissao, Y PARA CIMA, origem base-
//     esquerda -- a mesma convencao de Level::addPlatform e de todos os
//     objPos passados a Pipeline). Espelha EXACTAMENTE o calculo de
//     letterbox usado em Renderer::recordCommandBuffer (linhas ~169-184) e
//     a formula de NDC em base.vert (ndc.y invertido, tambem para UI sem
//     camara) -- um clique que caia visualmente sobre um rectangulo
//     desenhado em (x,y,w,h) tem de mapear para um ponto logico dentro
//     desse rectangulo. Testado com casos verificados a mao.
//
//     ATENCAO -- fonte duplicada de proposito: o calculo de letterbox
//     (offsetX/offsetY/viewW/viewH) existe hoje SO' dentro de Renderer.cpp,
//     inline. Nao foi extraido para aqui e partilhado por Renderer.cpp
//     (evitar tocar em codigo grafico estavel sem confirmacao explicita).
//     Se o calculo de Renderer.cpp mudar (ex: TARGET_ASPECT deixar de vir
//     de Config.h), este ficheiro tem de ser actualizado a par.
//
//  2. hitTestMenuBox(): dado um ponto logico, devolve qual das N caixas de
//     MENU/PAUSED foi atingida (ou -1). A partir da v9.3, Renderer.cpp
//     TAMBEM usa MenuBoxLayout::boxX()/boxWidth() directamente (deixou de
//     ter a sua propria copia inline destes valores) -- fonte unica de
//     verdade entre desenho e hit-test, resolvendo a duplicacao que a
//     v9.2 tinha deliberadamente deixado por resolver.
// =============================================================================

#include <cstdint>
#include <algorithm>

namespace core {

// ── Letterbox / conversao de coordenadas ─────────────────────────────────────

struct LogicalPoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Rectangulo do viewport letterboxed, em pixeis de janela.
struct LetterboxRect {
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    int32_t width   = 0;
    int32_t height  = 0;
};

// Calcula o rectangulo do viewport letterboxed dentro de uma janela
// windowWidth x windowHeight, para um rácio de aspecto alvo targetAspect
// (config::LOGICAL_WIDTH / config::LOGICAL_HEIGHT). Mesma formula usada em
// Renderer::recordCommandBuffer.
LetterboxRect computeLetterbox(int32_t windowWidth, int32_t windowHeight, float targetAspect);

// Converte uma posicao do cursor (pixeis de JANELA, Y para baixo) para
// espaco LOGICO do motor (Y para cima). Pontos fora do viewport letterboxed
// (ex: nas barras pretas) devolvem coordenadas fora de [0,logicalWidth) x
// [0,logicalHeight) -- deliberado, nao clampado: um clique nas barras nao
// deve acertar em nada, e nao acerta naturalmente em nenhum hit-test.
LogicalPoint windowToLogical(double windowX, double windowY,
                             int32_t windowWidth, int32_t windowHeight,
                             float logicalWidth, float logicalHeight);

// ── Hit-test do MENU/PAUSED (N caixas horizontais) ───────────────────────────

struct MenuBoxLayout {
    static constexpr float BOX_H    = 62.0f;
    static constexpr float BOX_Y    = 150.0f;
    static constexpr float GAP      = 18.0f;
    static constexpr float MAX_BOX_W = 170.0f; // largura quando ha' espaco de sobra (ex: 3 caixas)

    // Largura de cada caixa para `count` caixas caberem em `logicalWidth`
    // (com GAP entre elas), sem exceder MAX_BOX_W. Para count=3/logicalWidth
    // =640 isto da' exactamente 170 (o valor fixo anterior); para count=4
    // encolhe para caber.
    static float boxWidth(int count, float logicalWidth);

    // X da caixa `index` (0..count-1), centrado em logicalWidth.
    static float boxX(int index, int count, float logicalWidth);
};

// Indice da caixa (0..count-1) sob o ponto logico dado, ou -1 se nenhuma
// caixa foi atingida. logicalY segue Y-para-cima (BOX_Y e' a base da
// caixa, tal como addPlatform(x,y,w,h)).
int hitTestMenuBox(float logicalX, float logicalY, int count, float logicalWidth);

} // namespace core
