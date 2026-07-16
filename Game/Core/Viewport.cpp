// =============================================================================
//  Game/Core/Viewport.cpp
//
//  @version 9.3
// =============================================================================
#include "Core/Viewport.h"

namespace core {

LetterboxRect computeLetterbox(int32_t windowWidth, int32_t windowHeight, float targetAspect) {
    LetterboxRect r;
    if (windowWidth <= 0 || windowHeight <= 0 || targetAspect <= 0.0f) return r;

    float windowAspect = (float)windowWidth / (float)windowHeight;
    int32_t viewW = windowWidth, viewH = windowHeight;

    if (windowAspect > targetAspect) viewW = (int32_t)(windowHeight * targetAspect);
    else                             viewH = (int32_t)(windowWidth / targetAspect);

    r.offsetX = (windowWidth  - viewW) / 2;
    r.offsetY = (windowHeight - viewH) / 2;
    r.width   = viewW;
    r.height  = viewH;
    return r;
}

LogicalPoint windowToLogical(double windowX, double windowY,
                             int32_t windowWidth, int32_t windowHeight,
                             float logicalWidth, float logicalHeight) {
    LetterboxRect vp = computeLetterbox(windowWidth, windowHeight, logicalWidth / logicalHeight);
    if (vp.width <= 0 || vp.height <= 0) return {};

    // Pixeis de janela -> pixeis relativos ao viewport (desfaz o offset do letterbox).
    double viewportX = windowX - vp.offsetX;
    double viewportY = windowY - vp.offsetY;

    // Normalizado [0,1] dentro do viewport (ainda Y para baixo aqui).
    double normX = viewportX / (double)vp.width;
    double normY = viewportY / (double)vp.height;

    // Espaco logico: X directo, Y invertido (janela e' Y-para-baixo,
    // motor e' Y-para-cima -- mesma inversao de base.vert: ndc.y = -(...)).
    LogicalPoint p;
    p.x = (float)(normX * logicalWidth);
    p.y = (float)((1.0 - normY) * logicalHeight);
    return p;
}

float MenuBoxLayout::boxWidth(int count, float logicalWidth) {
    if (count <= 0) return 0.0f;
    float totalGaps  = GAP * (count - 1);
    float available  = logicalWidth - totalGaps;
    float w          = available / (float)count;
    return std::min(w, MAX_BOX_W);
}

float MenuBoxLayout::boxX(int index, int count, float logicalWidth) {
    float bW      = boxWidth(count, logicalWidth);
    float totalW  = bW * count + GAP * (count - 1);
    float cx      = logicalWidth / 2.0f;
    float startX  = cx - totalW / 2.0f;
    return startX + index * (bW + GAP);
}

int hitTestMenuBox(float logicalX, float logicalY, int count, float logicalWidth) {
    if (logicalY < MenuBoxLayout::BOX_Y || logicalY > MenuBoxLayout::BOX_Y + MenuBoxLayout::BOX_H)
        return -1;

    float bW = MenuBoxLayout::boxWidth(count, logicalWidth);
    for (int i = 0; i < count; ++i) {
        float x0 = MenuBoxLayout::boxX(i, count, logicalWidth);
        float x1 = x0 + bW;
        if (logicalX >= x0 && logicalX <= x1) return i;
    }
    return -1;
}

} // namespace core
