// =============================================================================
//  Tests/Unit/test_camera.cpp
//
//  @version 4.1
//  @history
//    v4.1 — criado (testes de conversão Ortográfica -> NDC)
// =============================================================================

#include "doctest/doctest.h"
#include "Graphics/Camera.h"

using namespace gfx;
using namespace logic;

TEST_SUITE("Fase 4.1 — Camera 2D e Viewport") {

    TEST_CASE("worldToNDC: Câmara na origem (0,0)") {
        Camera cam;
        cam.position = {0.0f, 0.0f};

        // Canto inferior esquerdo do mundo (0,0) deve mapear para NDC (-1.0, 1.0)
        Vec2 bottomLeft = cam.worldToNDC({0.0f, 0.0f});
        CHECK(bottomLeft.x == doctest::Approx(-1.0f));
        CHECK(bottomLeft.y == doctest::Approx(1.0f));

        // Canto superior direito do mundo (360, 640) deve mapear para NDC (1.0, -1.0)
        Vec2 topRight = cam.worldToNDC({Camera::LOGICAL_WIDTH, Camera::LOGICAL_HEIGHT});
        CHECK(topRight.x == doctest::Approx(1.0f));
        CHECK(topRight.y == doctest::Approx(-1.0f));

        // Centro do mundo deve ser exatamente (0.0, 0.0) em NDC
        Vec2 center = cam.worldToNDC({Camera::LOGICAL_WIDTH / 2.0f, Camera::LOGICAL_HEIGHT / 2.0f});
        CHECK(center.x == doctest::Approx(0.0f));
        CHECK(center.y == doctest::Approx(0.0f));
    }

    TEST_CASE("worldToNDC: Câmara deslocada (Tracking do Player)") {
        Camera cam;
        // Simulando a câmara a seguir o jogador (subiu 1000px e moveu-se 50px para a direita)
        cam.position = {50.0f, 1000.0f};

        // O ponto exato onde a câmara está agora representa o canto inferior esquerdo do ecrã
        Vec2 bottomLeft = cam.worldToNDC({50.0f, 1000.0f});
        CHECK(bottomLeft.x == doctest::Approx(-1.0f));
        CHECK(bottomLeft.y == doctest::Approx(1.0f));

        // Um inimigo na posição X=230, Y=1320 deve estar exatamente no centro do ecrã
        // (50 + 360/2 = 230)  |  (1000 + 640/2 = 1320)
        Vec2 center = cam.worldToNDC({230.0f, 1320.0f});
        CHECK(center.x == doctest::Approx(0.0f));
        CHECK(center.y == doctest::Approx(0.0f));
    }
}