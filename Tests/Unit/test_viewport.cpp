// =============================================================================
//  Tests/Unit/test_viewport.cpp
//
//  @version 9.3
//  @history
//    v9.2 — criado (Fase 9.2: suporte a rato)
//    v9.3 — hitTestMenuBox/MenuBoxLayout generalizados para N caixas;
//           count=3 reconfirmado identico a v9.2, count=4 adicionado
//           (Fase 9.3: MENU ganha a opcao "Editor").
//
//  SEM dependencias de GPU/GLFW -- pura matematica, com valores de
//  referencia calculados a mao a partir da MESMA formula usada em
//  Renderer::recordCommandBuffer (ver Viewport.h para a nota completa).
// =============================================================================
#include "doctest/doctest.h"
#include "Core/Viewport.h"
#include "Core/Config.h"

using namespace core;

TEST_SUITE("Fase 9.2 — Viewport: computeLetterbox") {

    TEST_CASE("janela com o mesmo racio do alvo -- sem barras") {
        // 1280x720 == exactamente 16:9 == config::TARGET_ASPECT (640/360)
        LetterboxRect r = computeLetterbox(1280, 720, config::TARGET_ASPECT);
        CHECK(r.offsetX == 0);
        CHECK(r.offsetY == 0);
        CHECK(r.width  == 1280);
        CHECK(r.height == 720);
    }

    TEST_CASE("janela mais larga que o alvo -- pillarbox (barras verticais)") {
        // 2560x1080, aspect 2.370 > 1.778 -> barras esquerda/direita
        LetterboxRect r = computeLetterbox(2560, 1080, config::TARGET_ASPECT);
        CHECK(r.height  == 1080);           // altura cheia
        CHECK(r.width   == 1920);           // 1080 * 16/9
        CHECK(r.offsetY == 0);
        CHECK(r.offsetX == 320);            // (2560-1920)/2
    }

    TEST_CASE("janela mais alta que o alvo -- letterbox (barras horizontais)") {
        // 1080x1920 (retrato), aspect 0.5625 < 1.778 -> barras cima/baixo
        LetterboxRect r = computeLetterbox(1080, 1920, config::TARGET_ASPECT);
        CHECK(r.width   == 1080);           // largura cheia
        CHECK(r.height  == 607);            // 1080 / 1.77778, truncado
        CHECK(r.offsetX == 0);
        CHECK(r.offsetY == 656);            // (1920-607)/2
    }

    TEST_CASE("dimensoes invalidas devolvem rectangulo vazio, sem crash") {
        LetterboxRect r1 = computeLetterbox(0, 720, config::TARGET_ASPECT);
        CHECK(r1.width == 0);
        LetterboxRect r2 = computeLetterbox(1280, 0, config::TARGET_ASPECT);
        CHECK(r2.width == 0);
    }
}

TEST_SUITE("Fase 9.2 — Viewport: windowToLogical") {

    TEST_CASE("janela sem barras: cantos e centro mapeiam correctamente (Y invertido)") {
        // 1280x720, sem letterbox -- mapeamento 1:1 simples (escala 2x)
        LogicalPoint topLeft = windowToLogical(0.0, 0.0, 1280, 720,
                                                config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
        // topo-esquerdo da JANELA (Y-baixo) -> topo-esquerdo LOGICO (Y-cima) = (0, LOGICAL_HEIGHT)
        CHECK(topLeft.x == doctest::Approx(0.0f));
        CHECK(topLeft.y == doctest::Approx(config::LOGICAL_HEIGHT));

        LogicalPoint bottomRight = windowToLogical(1280.0, 720.0, 1280, 720,
                                                    config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
        // baixo-direita da JANELA -> baixo-direita LOGICO = (LOGICAL_WIDTH, 0)
        CHECK(bottomRight.x == doctest::Approx(config::LOGICAL_WIDTH));
        CHECK(bottomRight.y == doctest::Approx(0.0f));

        LogicalPoint center = windowToLogical(640.0, 360.0, 1280, 720,
                                              config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
        CHECK(center.x == doctest::Approx(config::LOGICAL_WIDTH  / 2.0f));
        CHECK(center.y == doctest::Approx(config::LOGICAL_HEIGHT / 2.0f));
    }

    TEST_CASE("clique dentro da barra de pillarbox cai fora do intervalo logico (sem clamp)") {
        // 2560x1080 pillarboxed: viewport = x[320,2240], barra esquerda = x[0,320)
        LogicalPoint p = windowToLogical(100.0, 500.0, 2560, 1080,
                                         config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
        CHECK(p.x < 0.0f); // fora de [0, LOGICAL_WIDTH) -- deliberado, nao clampado
    }

    TEST_CASE("clique no canto superior-esquerdo do viewport (dentro da barra) mapeia para logico (0, LOGICAL_HEIGHT)") {
        // 2560x1080 pillarboxed: canto superior-esquerdo do VIEWPORT (nao da janela) fica em x=320,y=0
        LogicalPoint p = windowToLogical(320.0, 0.0, 2560, 1080,
                                         config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
        CHECK(p.x == doctest::Approx(0.0f));
        CHECK(p.y == doctest::Approx(config::LOGICAL_HEIGHT));
    }
}

TEST_SUITE("Fase 9.2/9.3 — Viewport: hitTestMenuBox") {
    // v9.3: MenuBoxLayout generalizado para N caixas (antes fixo em 3).
    // Valores de referencia calculados a mao a partir da formula em
    // MenuBoxLayout::boxWidth/boxX, para LOGICAL_WIDTH=640:
    //   count=3 (PAUSED, inalterado): box0=[47,217] box1=[235,405] box2=[423,593], largura 170
    //   count=4 (MENU, novo Fase 9.3): box0=[0,146.5] box1=[164.5,311] box2=[329,475.5] box3=[493.5,640], largura 146.5

    TEST_CASE("count=3: boxWidth e boxX reproduzem EXACTAMENTE os valores da v9.2 (sem regressao no PAUSED)") {
        CHECK(MenuBoxLayout::boxWidth(3, config::LOGICAL_WIDTH) == doctest::Approx(170.0f));
        CHECK(MenuBoxLayout::boxX(0, 3, config::LOGICAL_WIDTH) == doctest::Approx(47.0f));
        CHECK(MenuBoxLayout::boxX(1, 3, config::LOGICAL_WIDTH) == doctest::Approx(235.0f));
        CHECK(MenuBoxLayout::boxX(2, 3, config::LOGICAL_WIDTH) == doctest::Approx(423.0f));
    }

    TEST_CASE("count=3: pontos dentro de cada caixa devolvem o indice correcto") {
        CHECK(hitTestMenuBox(100.0f, 180.0f, 3, config::LOGICAL_WIDTH) == 0);
        CHECK(hitTestMenuBox(300.0f, 180.0f, 3, config::LOGICAL_WIDTH) == 1);
        CHECK(hitTestMenuBox(500.0f, 180.0f, 3, config::LOGICAL_WIDTH) == 2);
    }

    TEST_CASE("count=3: pontos no gap entre caixas devolvem -1") {
        CHECK(hitTestMenuBox(225.0f, 180.0f, 3, config::LOGICAL_WIDTH) == -1);
        CHECK(hitTestMenuBox(413.0f, 180.0f, 3, config::LOGICAL_WIDTH) == -1);
    }

    TEST_CASE("count=3: pontos fora do intervalo Y ou lateralmente fora devolvem -1") {
        CHECK(hitTestMenuBox(300.0f, 100.0f, 3, config::LOGICAL_WIDTH) == -1);
        CHECK(hitTestMenuBox(300.0f, 250.0f, 3, config::LOGICAL_WIDTH) == -1);
        CHECK(hitTestMenuBox(10.0f,  180.0f, 3, config::LOGICAL_WIDTH) == -1);
        CHECK(hitTestMenuBox(620.0f, 180.0f, 3, config::LOGICAL_WIDTH) == -1);
    }

    TEST_CASE("count=3: limites exactos das caixas sao inclusivos") {
        CHECK(hitTestMenuBox(47.0f,  180.0f, 3, config::LOGICAL_WIDTH) == 0);
        CHECK(hitTestMenuBox(217.0f, 180.0f, 3, config::LOGICAL_WIDTH) == 0);
        CHECK(hitTestMenuBox(300.0f, 150.0f, 3, config::LOGICAL_WIDTH) == 1);
        CHECK(hitTestMenuBox(300.0f, 212.0f, 3, config::LOGICAL_WIDTH) == 1);
    }

    TEST_CASE("count=4 (MENU, Fase 9.3): boxWidth encolhe para caber, boxX calcula as 4 posicoes") {
        CHECK(MenuBoxLayout::boxWidth(4, config::LOGICAL_WIDTH) == doctest::Approx(146.5f));
        CHECK(MenuBoxLayout::boxX(0, 4, config::LOGICAL_WIDTH) == doctest::Approx(0.0f));
        CHECK(MenuBoxLayout::boxX(1, 4, config::LOGICAL_WIDTH) == doctest::Approx(164.5f));
        CHECK(MenuBoxLayout::boxX(2, 4, config::LOGICAL_WIDTH) == doctest::Approx(329.0f));
        CHECK(MenuBoxLayout::boxX(3, 4, config::LOGICAL_WIDTH) == doctest::Approx(493.5f));
    }

    TEST_CASE("count=4: pontos dentro de cada uma das 4 caixas devolvem o indice correcto") {
        CHECK(hitTestMenuBox(50.0f,  180.0f, 4, config::LOGICAL_WIDTH) == 0);
        CHECK(hitTestMenuBox(200.0f, 180.0f, 4, config::LOGICAL_WIDTH) == 1);
        CHECK(hitTestMenuBox(400.0f, 180.0f, 4, config::LOGICAL_WIDTH) == 2);
        CHECK(hitTestMenuBox(600.0f, 180.0f, 4, config::LOGICAL_WIDTH) == 3);
    }

    TEST_CASE("count=4: gap entre caixas continua a devolver -1") {
        CHECK(hitTestMenuBox(155.0f, 180.0f, 4, config::LOGICAL_WIDTH) == -1); // entre box0 e box1
    }

    TEST_CASE("count invalido (<=0) nao rebenta -- boxWidth devolve 0") {
        CHECK(MenuBoxLayout::boxWidth(0, config::LOGICAL_WIDTH) == doctest::Approx(0.0f));
    }
}
