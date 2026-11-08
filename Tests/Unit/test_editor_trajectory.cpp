// =============================================================================
//  Tests/Unit/test_editor_trajectory.cpp
//
//  Validação da Pré-visualização de Trajetórias Balísticas no Editor
//  (Baseado em Aramini et al. 2018 e Smith et al. 2008)
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Logic/EditorSession.h"
#include "Logic/JumpTrajectory.h"
#include "Core/Config.h"

#include <cmath>

TEST_SUITE("Editor — Trajectory Preview") {

TEST_CASE("EditorSession gera trajetorias analiticas para plataforma selecionada") {
    logic::EditorSession session(false);
    // Adicionar plataforma de teste via document
    session.document().addPlatform(logic::AABB{{100.0f, 50.0f}, {200.0f, 66.0f}});

    // Selecionar a primeira plataforma via controller
    session.controller().selectPlatform(0);

    const auto snapshot = session.renderSnapshot();

    CHECK(snapshot.hasSelection == true);
    CHECK(snapshot.selectedIndex == 0);
    CHECK(snapshot.hasTrajectoryPreview == true);

    CHECK_FALSE(snapshot.trajectoryMinRight.empty());
    CHECK_FALSE(snapshot.trajectoryMaxRight.empty());
    CHECK_FALSE(snapshot.trajectoryMinLeft.empty());
    CHECK_FALSE(snapshot.trajectoryMaxLeft.empty());

    // Ponto de lançamento deve ser o topo-centro da plataforma:
    // x = (100 + 200) / 2 = 150, y = 66
    const logic::Vec2 expectedLaunch{150.0f, 66.0f};
    CHECK(snapshot.trajectoryMinRight.front().position.x == doctest::Approx(expectedLaunch.x));
    CHECK(snapshot.trajectoryMinRight.front().position.y == doctest::Approx(expectedLaunch.y));
    CHECK(snapshot.trajectoryMaxRight.front().position.x == doctest::Approx(expectedLaunch.x));
    CHECK(snapshot.trajectoryMaxRight.front().position.y == doctest::Approx(expectedLaunch.y));

    // Trajetória máxima deve atingir altura consideravelmente superior à mínima
    float maxApexY = expectedLaunch.y;
    float minApexY = expectedLaunch.y;

    for (const auto& pt : snapshot.trajectoryMinRight) {
        CHECK(std::isfinite(pt.position.x));
        CHECK(std::isfinite(pt.position.y));
        if (pt.position.y > minApexY) minApexY = pt.position.y;
    }
    for (const auto& pt : snapshot.trajectoryMaxRight) {
        CHECK(std::isfinite(pt.position.x));
        CHECK(std::isfinite(pt.position.y));
        if (pt.position.y > maxApexY) maxApexY = pt.position.y;
    }

    CHECK(maxApexY > minApexY);
    // Altura do salto mínimo analítico ~ 23.9px
    CHECK((minApexY - expectedLaunch.y) == doctest::Approx(23.9f).epsilon(0.05f));
    // Altura do salto máximo analítico ~ 137.7px
    CHECK((maxApexY - expectedLaunch.y) == doctest::Approx(137.7f).epsilon(0.05f));
}

TEST_CASE("EditorSession gera trajetoria a partir do spawn quando nada esta selecionado") {
    logic::EditorSession session(false);
    session.controller().clearSelection();

    const auto snapshot = session.renderSnapshot();

    CHECK(snapshot.hasSelection == false);
    CHECK(snapshot.hasTrajectoryPreview == true);

    const logic::Vec2 spawn = snapshot.spawnPosition;
    CHECK(snapshot.trajectoryMinRight.front().position.x == doctest::Approx(spawn.x));
    CHECK(snapshot.trajectoryMinRight.front().position.y == doctest::Approx(spawn.y));
}

} // TEST_SUITE
