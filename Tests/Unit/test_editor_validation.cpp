#include "doctest/doctest.h"
#include "Logic/EditorSession.h"
#include "Logic/EditorValidation.h"
#include "Logic/LevelEditorValidator.h"

#include <chrono>
#include <thread>

TEST_SUITE("Fase 9.5 — EditorValidationTask") {

TEST_CASE("validação assíncrona devolve diagnóstico físico e caminho exato") {
    logic::LevelData data;
    data.name = "Async Validation";
    data.finalCampaignLevel = true;
    data.platforms.push_back(logic::AABB{{256.0f, 80.0f}, {384.0f, 100.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, 7, "custom/test.lvl"));

    while (task.running())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    const auto result = task.poll();
    CHECK(result.state == logic::EditorValidationState::COMPLETE);
    CHECK(result.valid);
    CHECK(result.reachesGoal);
    CHECK(result.reachablePlatforms == 1);
    CHECK(result.totalPlatforms == 1);
    REQUIRE(result.platformReachable.size() == 1);
    CHECK(result.platformReachable[0]);
    CHECK(result.generation == 7);
    CHECK(result.levelPath == "custom/test.lvl");
    CHECK(result.message == "CAMINHO OK — objetivo alcancavel");
}

TEST_CASE("validação física identifica plataforma inalcançável") {
    logic::LevelData data;
    data.name = "Physical Invalid";
    data.finalCampaignLevel = true;
    data.platforms.push_back(logic::AABB{{256.0f, 80.0f}, {384.0f, 100.0f}});
    data.platforms.push_back(logic::AABB{{0.0f, 340.0f}, {128.0f, 360.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, 12, "invalid/test.lvl"));

    while (task.running())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    const auto result = task.poll();
    CHECK(result.state == logic::EditorValidationState::COMPLETE);
    CHECK_FALSE(result.valid);
    CHECK_FALSE(result.reachesGoal);
    CHECK(result.reachablePlatforms == 1);
    CHECK(result.totalPlatforms == 2);
    REQUIRE(result.platformReachable.size() == 2);
    CHECK(result.platformReachable[0]);
    CHECK_FALSE(result.platformReachable[1]);
    CHECK(result.generation == 12);
    CHECK(result.levelPath == "invalid/test.lvl");
}

TEST_CASE("nova validação não pode substituir uma tarefa ainda em execução") {
    logic::LevelData data;
    data.finalCampaignLevel = true;
    data.platforms.push_back(logic::AABB{{256.0f, 80.0f}, {384.0f, 100.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, 1, "first.lvl"));
    CHECK_FALSE(task.start(data, 2, "second.lvl"));
    task.discard();
}

TEST_CASE("validator reporta reachability por plataforma") {
    logic::EditorSession session(true);
    REQUIRE(session.document().addPlatform({{256.0f, 80.0f}, {384.0f, 100.0f}}));
    REQUIRE(session.document().addPlatform({{0.0f, 340.0f}, {128.0f, 360.0f}}));

    const logic::EditorValidationResult result =
        logic::validateEditorDocument(session.document());

    REQUIRE(result.totalPlatforms == 2);
    REQUIRE(result.platformReachable.size() == 2);
    CHECK(result.platformReachable[0]);
    CHECK_FALSE(result.platformReachable[1]);
    CHECK(result.reachablePlatforms == 1);
    CHECK_FALSE(result.reachesGoal);
    CHECK_FALSE(result.valid);
}

TEST_CASE("metadata de validação não é serializado como conteúdo authored") {
    logic::EditorSession session(true);
    const logic::LevelData data = session.document().toLevelData("Test");

    CHECK(data.finalCampaignLevel);
    CHECK_FALSE(data.flag.has_value());
    CHECK_FALSE(data.spawnPosition.has_value());
}

} // TEST_SUITE
