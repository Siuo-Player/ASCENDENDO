// =============================================================================
//  Tests/Unit/test_camera.cpp
//
//  @version 6.4
//  @history
//    v4.1  — worldToNDC (projecao world→NDC, Y invertido para Vulkan)
//    v6.4  — follow() (tracking vertical suave)
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/Camera.h"
#include "Logic/Physics.h"
#include "Core/Config.h"

using namespace gfx;
using namespace logic;

TEST_SUITE("Fase 4.1 — Camera 2D e Viewport") {

    TEST_CASE("worldToNDC: Camara na origem (0,0)") {
        Camera cam;

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

    TEST_CASE("worldToNDC: Camara deslocada (Tracking do Player)") {
        Camera cam;
        cam.position.y = config::LOGICAL_HEIGHT / 2.0f; // camera a Y=180

        // Canto inferior-esquerdo do viewport = posicao da camera
        Vec2 bottomLeft = cam.worldToNDC({0.0f, cam.position.y});
        CHECK(bottomLeft.x == doctest::Approx(-1.0f));
        CHECK(bottomLeft.y == doctest::Approx(1.0f));

        // Centro do viewport em world space
        Vec2 center = cam.worldToNDC({config::LOGICAL_WIDTH  / 2.0f,
                                       cam.position.y + config::LOGICAL_HEIGHT / 2.0f});
        CHECK(center.x == doctest::Approx(0.0f));
        CHECK(center.y == doctest::Approx(0.0f));
    }
}

TEST_SUITE("Fase 6.4 — Camera Tracking Vertical") {

    TEST_CASE("follow() move camera em direcao ao jogador") {
        Camera cam;
        cam.position = {0.0f, 0.0f};

        // Jogador a Y=400 → alvo da camera = 400 - 126 = 274
        Vec2 playerPos = {320.0f, 400.0f};
        constexpr float target = 400.0f - config::LOGICAL_HEIGHT * 0.35f; // 274

        // Apos muitos frames (2s a 60fps) a camera deve estar proxima do alvo
        for (int i = 0; i < 120; ++i)
            cam.follow(playerPos, PhysicsWorld::FIXED_STEP);

        CHECK(cam.position.y > target * 0.9f);  // chegou a 90% ou mais
        CHECK(cam.position.y <= target + 1.0f); // nao ultrapassou
    }

    TEST_CASE("follow() nao vai abaixo de zero") {
        Camera cam;
        cam.position = {0.0f, 50.0f};

        // Jogador perto do chao → alvo = max(0, 10-126) = 0
        Vec2 playerPos = {320.0f, 10.0f};

        for (int i = 0; i < 120; ++i)
            cam.follow(playerPos, PhysicsWorld::FIXED_STEP);

        CHECK(cam.position.y >= 0.0f);
        CHECK(cam.position.y < 1.0f); // convergiu para ~0
    }

    TEST_CASE("follow() nao move camera em X") {
        Camera cam;
        cam.position = {0.0f, 0.0f};

        Vec2 playerPos = {500.0f, 300.0f}; // jogador deslocado em X
        cam.follow(playerPos, PhysicsWorld::FIXED_STEP);

        CHECK(cam.position.x == doctest::Approx(0.0f)); // X inalterado
    }
}
