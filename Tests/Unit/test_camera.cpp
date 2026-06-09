// =============================================================================
//  Tests/Unit/test_camera.cpp
//
//  @version 4.1
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/Camera.h"
#include "Core/Config.h" // <- Ler as globais

using namespace gfx;
using namespace logic;

TEST_SUITE("Fase 4.1 — Camera 2D e Viewport") {
    TEST_CASE("worldToNDC: Câmara na origem (0,0)") {
        Camera cam;
        cam.position = {0.0f, 0.0f};

        Vec2 bottomLeft = cam.worldToNDC({0.0f, 0.0f});
        CHECK(bottomLeft.x == doctest::Approx(-1.0f));
        CHECK(bottomLeft.y == doctest::Approx(1.0f));

        Vec2 topRight = cam.worldToNDC({config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT});
        CHECK(topRight.x == doctest::Approx(1.0f));
        CHECK(topRight.y == doctest::Approx(-1.0f));

        Vec2 center = cam.worldToNDC({config::LOGICAL_WIDTH / 2.0f, config::LOGICAL_HEIGHT / 2.0f});
        CHECK(center.x == doctest::Approx(0.0f));
        CHECK(center.y == doctest::Approx(0.0f));
    }

    TEST_CASE("worldToNDC: Câmara deslocada (Tracking do Player)") {
        Camera cam;
        cam.position = {50.0f, 1000.0f};

        Vec2 bottomLeft = cam.worldToNDC({50.0f, 1000.0f});
        CHECK(bottomLeft.x == doctest::Approx(-1.0f));
        CHECK(bottomLeft.y == doctest::Approx(1.0f));

        // Centro agora é: X = 50 + (640/2) = 370 | Y = 1000 + (360/2) = 1180
        Vec2 center = cam.worldToNDC({370.0f, 1180.0f});
        CHECK(center.x == doctest::Approx(0.0f));
        CHECK(center.y == doctest::Approx(0.0f));
    }
}