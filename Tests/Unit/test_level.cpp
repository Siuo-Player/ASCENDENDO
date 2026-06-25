// =============================================================================
//  Tests/Unit/test_level.cpp
//
//  @version 7.4
//  @history
//    v6.2  — criado (10 testes)
//    v6.2c — +3 testes colisao lateral (MTV + bounce elastico)
//    v7.2  — refatorado: tetos solidos, ajuste width=16, suites renomeadas
//    v7.3  — +4 testes Level::appendFromFile (carregamento de ficheiros .lvl)
//    v7.4  — FIX: appendFromFile agora retorna SEMPRE offsetY+LOGICAL_HEIGHT
//            (avanço padronizado a uma tela, antes era highestY+50 — variavel).
//            Teste de stacking corrigido (360, nao 165). FLAG de teste movida
//            para y=300 (dentro de uma tela). +1 teste de regressao explicito:
//            nivel vazio e nivel cheio avançam o MESMO valor.
//
//  SEM dependencias de GPU. Testa o logic layer isolado.
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include <fstream>
#include <cstdio>   // std::remove

using namespace logic;

// ── Helper ───────────────────────────────────────────────────────────────────
static void simulate(PhysicsBody& body, PhysicsWorld& world,
                     const Level& level, int steps)
{
    for (int i = 0; i < steps; ++i) {
        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  [v7.2] Suites originais — inalteradas
// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Construcao") {

    TEST_CASE("nivel vazio tem zero plataformas") {
        Level level;
        CHECK(level.platformCount() == 0);
        CHECK(level.platforms().empty());
    }

    TEST_CASE("addPlatform cria plataforma com bounds corretos") {
        Level level;
        level.addPlatform(50.0f, 100.0f, 200.0f, 20.0f);

        REQUIRE(level.platformCount() == 1);
        const auto& p = level.platforms()[0];
        CHECK(p.bounds.min.x == doctest::Approx(50.0f));
        CHECK(p.bounds.min.y == doctest::Approx(100.0f));
        CHECK(p.bounds.max.x == doctest::Approx(250.0f));
        CHECK(p.bounds.max.y == doctest::Approx(120.0f));
    }

    TEST_CASE("multiplas plataformas guardadas na ordem de insercao") {
        Level level;
        level.addPlatform(0.0f,  50.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 150.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 250.0f, 100.0f, 10.0f);

        REQUIRE(level.platformCount() == 3);
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(50.0f));
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(150.0f));
        CHECK(level.platforms()[2].bounds.min.y == doctest::Approx(250.0f));
    }

    TEST_CASE("clear() remove todas as plataformas") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 200.0f, 100.0f, 10.0f);
        REQUIRE(level.platformCount() == 2);
        level.clear();
        CHECK(level.platformCount() == 0);
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Colisao Vertical (Topo e Teto)") {

    TEST_CASE("corpo cai e pousa no topo da plataforma") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f);

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {100.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(120.0f));
    }

    TEST_CASE("corpo a subir bate no teto (solid blocks - v6.6)") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f);

        PhysicsWorld world;
        PhysicsBody  body;
        // AJUSTE: Subido de 65.0f para 80.0f para o corpo de 16px alcançar o teto
        body.position   = {50.0f, 80.0f};
        body.velocity.y = 300.0f;
        body.velocity.x = 100.0f;
        body.isGrounded = false;

        world.step(body, PhysicsWorld::FIXED_STEP);
        bool collided = level.resolveCollision(body);

        CHECK(collided == true);
        CHECK(body.isGrounded == false);
        CHECK(body.position.y == doctest::Approx(100.0f - body.height));

        float expectedVy = (300.0f + (PhysicsWorld::GRAVITY * PhysicsWorld::FIXED_STEP)) * -0.3f;
        CHECK(body.velocity.y == doctest::Approx(expectedVy));
        CHECK(body.velocity.x == doctest::Approx(90.0f));
    }

    TEST_CASE("nivel vazio nao interfere com colisao do chao") {
        Level level;
        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {0.0f, 50.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(0.0f));
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Fisica com Nivel") {

    TEST_CASE("plataforma mais alta intercepta antes do chao") {
        Level level;
        level.addPlatform(0.0f, 200.0f, 640.0f, 20.0f);

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {100.0f, 400.0f};
        body.isGrounded = false;

        simulate(body, world, level, 300);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(220.0f));
    }

    TEST_CASE("corpo cai apos sair lateralmente da plataforma") {
        Level level;
        level.addPlatform(100.0f, 100.0f, 100.0f, 20.0f);

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {120.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);
        REQUIRE(body.isGrounded == true);

        body.position.x = 350.0f;
        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);

        CHECK(body.isGrounded == false);
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Colisao Lateral") {

    TEST_CASE("corpo bloqueado ao entrar pela esquerda da plataforma") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);

        PhysicsBody body;
        // AJUSTE: Movido de 73.0f para 89.0f para colmatar a nova largura de 16px
        body.position   = {89.0f, 70.0f};
        body.velocity.x = 200.0f;
        body.velocity.y = 0.0f;
        body.isGrounded = true;

        bool collided = level.resolveCollision(body);

        CHECK(collided == true);
        CHECK(body.velocity.x == doctest::Approx(-60.0f));
        CHECK(body.position.x + body.width <= doctest::Approx(100.0f));
    }

    TEST_CASE("corpo bloqueado ao entrar pela direita da plataforma") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);

        PhysicsBody body;
        body.position   = {195.0f, 70.0f};
        body.velocity.x = -200.0f;
        body.velocity.y = 0.0f;
        body.isGrounded = true;

        bool collided = level.resolveCollision(body);

        CHECK(collided == true);
        CHECK(body.velocity.x == doctest::Approx(60.0f));
        CHECK(body.position.x == doctest::Approx(200.0f));
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  [v7.3] Novos testes — Level::appendFromFile
// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / File Loading") {
    // appendFromFile imprime [ERRO] em stderr se o ficheiro nao existir — esperado.

    TEST_CASE("appendFromFile: ficheiro inexistente retorna offsetY inalterado") {
        Level level;
        float result = level.appendFromFile("nao_existe_mesmo.lvl", 640.0f, 0.0f);

        // Sem crash, sem alteracoes
        CHECK(result == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
    }

    TEST_CASE("appendFromFile: PLATFORM carregada com offset Y correto") {
        const std::string tmpPath = "tmp_lvl_platform.lvl";
        {
            std::ofstream f(tmpPath);
            f << "# teste temporario\n";
            f << "NAME TestPlatform\n";
            f << "PLATFORM 50.0 100.0 100.0 15.0\n";
        }

        Level level;
        level.appendFromFile(tmpPath, 640.0f, 0.0f);

        REQUIRE(level.platformCount() == 1);
        CHECK(level.platforms()[0].bounds.min.x == doctest::Approx(50.0f));
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(100.0f));
        CHECK(level.platforms()[0].bounds.max.y == doctest::Approx(115.0f));

        std::remove(tmpPath.c_str());
    }

    TEST_CASE("appendFromFile: FLAG define hasFlag e flagBounds") {
        const std::string tmpPath = "tmp_lvl_flag.lvl";
        {
            std::ofstream f(tmpPath);
            f << "FLAG 150.0 300.0 40.0 40.0\n";
        }

        Level level;
        level.appendFromFile(tmpPath, 640.0f, 0.0f);

        CHECK(level.hasFlag == true);
        CHECK(level.flagBounds.min.x == doctest::Approx(150.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(300.0f));
        CHECK(level.flagBounds.max.x == doctest::Approx(190.0f));
        CHECK(level.flagBounds.max.y == doctest::Approx(340.0f));

        std::remove(tmpPath.c_str());
    }

    TEST_CASE("appendFromFile: segundo chunk usa offsetY = offsetY_anterior + LOGICAL_HEIGHT") {
        // v7.2: o avanço de chunk é SEMPRE padronizado a uma tela de altura,
        // independentemente do conteudo do nivel (highestY deixou de ser usado
        // no calculo do retorno).
        const std::string tmpPath = "tmp_lvl_stack.lvl";
        {
            std::ofstream f(tmpPath);
            f << "PLATFORM 0.0 100.0 200.0 15.0\n";
        }

        Level level;
        float nextY = level.appendFromFile(tmpPath, 640.0f, 0.0f);
        // Retorno padronizado: 0 + LOGICAL_HEIGHT (360), NAO depende de highestY
        CHECK(nextY == doctest::Approx(360.0f));

        // Segundo chunk: plataforma local y=100 deve aparecer em 360 + 100 = 460
        level.appendFromFile(tmpPath, 640.0f, nextY);
        REQUIRE(level.platformCount() == 2);
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(460.0f));

        std::remove(tmpPath.c_str());
    }

    TEST_CASE("appendFromFile: avanco do chunk e identico mesmo sem plataformas") {
        // Regressao do bug: antes, um nivel "vazio" ou raso avancava muito menos
        // que um nivel cheio (highestY+50), quebrando o trigger de streaming em
        // main.cpp. Agora ambos avancam exactamente LOGICAL_HEIGHT.
        const std::string tmpEmpty = "tmp_lvl_empty.lvl";
        const std::string tmpFull  = "tmp_lvl_full.lvl";
        {
            std::ofstream f(tmpEmpty);
            f << "# nivel sem plataformas\n";
        }
        {
            std::ofstream f(tmpFull);
            f << "PLATFORM 0.0 0.0 640.0 20.0\n";
            f << "PLATFORM 230.0 95.0 180.0 20.0\n";
            f << "PLATFORM 30.0 195.0 180.0 20.0\n";
            f << "PLATFORM 230.0 295.0 180.0 20.0\n"; // topo local = 315, < 360
        }

        Level levelEmpty, levelFull;
        float nextEmpty = levelEmpty.appendFromFile(tmpEmpty, 640.0f, 0.0f);
        float nextFull  = levelFull.appendFromFile(tmpFull,  640.0f, 0.0f);

        CHECK(nextEmpty == doctest::Approx(360.0f));
        CHECK(nextFull  == doctest::Approx(360.0f));
        CHECK(nextEmpty == doctest::Approx(nextFull)); // o ponto crucial do fix

        std::remove(tmpEmpty.c_str());
        std::remove(tmpFull.c_str());
    }
}
