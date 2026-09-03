#include "doctest/doctest.h"
#include "Logic/EditorValidation.h"

#include <chrono>
#include <thread>

TEST_SUITE("Fase 9.5 — EditorValidationTask") {

TEST_CASE("validação assíncrona usa snapshot imutável e devolve o nível exato") {
    logic::LevelData data;
    data.name = "Async Validation";
    data.spawnPosition = logic::Vec2{32.0f, 20.0f};
    data.platforms.push_back(logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, "custom/test.lvl"));
    CHECK(task.running());

    logic::EditorValidationResult result;
    for (int i = 0; i < 200 && task.running(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    result = task.poll();
    CHECK(result.state == logic::EditorValidationState::COMPLETE);
    CHECK(result.valid);
    CHECK(result.levelPath == "custom/test.lvl");
    CHECK(result.message == "Representação semântica válida");
}

TEST_CASE("validação rejeita geometria semanticamente inválida sem bloquear o editor") {
    logic::LevelData data;
    data.name = "Invalid Validation";
    data.platforms.push_back(logic::AABB{{10.0f, 0.0f}, {10.0f, 20.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, "invalid/test.lvl"));

    while (task.running())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    const auto result = task.poll();
    CHECK(result.state == logic::EditorValidationState::COMPLETE);
    CHECK_FALSE(result.valid);
    CHECK(result.levelPath == "invalid/test.lvl");
}

TEST_CASE("nova validação não pode substituir uma tarefa ainda em execução") {
    logic::LevelData data;
    data.platforms.push_back(logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, "first.lvl"));
    CHECK_FALSE(task.start(data, "second.lvl"));
    task.discard();
}

} // TEST_SUITE
