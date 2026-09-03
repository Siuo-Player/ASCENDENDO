#include "doctest/doctest.h"
#include "Logic/EditorValidation.h"

#include <chrono>
#include <thread>

TEST_SUITE("Fase 9.5 — EditorValidationTask") {

TEST_CASE("validação assíncrona usa snapshot imutável, geração e nível exato") {
    logic::LevelData data;
    data.name = "Async Validation";
    data.spawnPosition = logic::Vec2{32.0f, 20.0f};
    data.platforms.push_back(logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, 7, "custom/test.lvl"));

    logic::EditorAsyncValidationResult result;
    for (int i = 0; i < 200 && task.running(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    result = task.poll();
    CHECK(result.state == logic::EditorValidationState::COMPLETE);
    CHECK(result.valid);
    CHECK(result.generation == 7);
    CHECK(result.levelPath == "custom/test.lvl");
    CHECK(result.message == "Representação semântica válida");
}

TEST_CASE("validação rejeita geometria semanticamente inválida sem bloquear o editor") {
    logic::LevelData data;
    data.name = "Invalid Validation";
    data.platforms.push_back(logic::AABB{{10.0f, 0.0f}, {10.0f, 20.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, 12, "invalid/test.lvl"));

    while (task.running())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    const auto result = task.poll();
    CHECK(result.state == logic::EditorValidationState::COMPLETE);
    CHECK_FALSE(result.valid);
    CHECK(result.generation == 12);
    CHECK(result.levelPath == "invalid/test.lvl");
}

TEST_CASE("nova validação não pode substituir uma tarefa ainda em execução") {
    logic::LevelData data;
    data.platforms.push_back(logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}});

    logic::EditorValidationTask task;
    REQUIRE(task.start(data, 1, "first.lvl"));
    CHECK_FALSE(task.start(data, 2, "second.lvl"));
    task.discard();
}

} // TEST_SUITE