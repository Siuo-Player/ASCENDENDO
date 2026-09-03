#include "doctest/doctest.h"
#include "Logic/EditorSession.h"

#include <chrono>
#include <filesystem>
#include <thread>

namespace {

logic::AABB reachableGoal() {
    return {{32.0f, 20.0f}, {48.0f, 36.0f}};
}

} // namespace

TEST_SUITE("Fase 9.5 — EditorSession persistence") {

TEST_CASE("saveLevel valida antes de escrever e grava um nível válido") {
    const auto path =
        std::filesystem::temp_directory_path() / "ascendendo-session-save-valid.lvl";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    logic::EditorSession session(true);
    REQUIRE(session.document().setFlag(reachableGoal()));

    const auto result = session.saveLevel(path.string(), "Session Save");
    CHECK(result.success);
    CHECK(result.validationPassed);
    CHECK(result.generation == session.documentGeneration());
    CHECK(result.path == path.string());
    CHECK(std::filesystem::exists(path));

    std::filesystem::remove(path, ec);
}

TEST_CASE("saveLevel recusa nível inválido sem criar o ficheiro") {
    const auto path =
        std::filesystem::temp_directory_path() / "ascendendo-session-save-invalid.lvl";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    logic::EditorSession session(true);
    const auto result = session.saveLevel(path.string(), "Invalid Session Save");

    CHECK_FALSE(result.success);
    CHECK_FALSE(result.validationPassed);
    CHECK(result.generation == session.documentGeneration());
    CHECK_FALSE(std::filesystem::exists(path));
}

TEST_CASE("EditorSave usa o target configurado pelo editor") {
    const auto path =
        std::filesystem::temp_directory_path() / "ascendendo-session-action-save.lvl";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    logic::EditorSession session(true);
    session.setPersistenceTarget(path.string(), "Action Save");
    REQUIRE(session.document().setFlag(reachableGoal()));

    logic::InputManager input;
    core::KeyBindings bindings;
    input.beginFrame();
    input.onKeyEvent(logic::Key::NUM1, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    CHECK(session.lastSaveResult().success);
    CHECK(std::filesystem::exists(path));
    std::filesystem::remove(path, ec);
}

TEST_CASE("resultado assíncrono fica STALE quando o documento muda depois do snapshot") {
    logic::EditorSession session(true);
    session.setPersistenceTarget("stale-validation.lvl", "Stale Validation");
    REQUIRE(session.document().setFlag(reachableGoal()));

    const std::uint64_t generationBefore = session.documentGeneration();
    REQUIRE(session.startValidation());

    REQUIRE(session.document().addPlatform({{96.0f, 80.0f}, {224.0f, 100.0f}}));
    CHECK(session.documentGeneration() > generationBefore);

    logic::EditorAsyncValidationResult result;
    for (int i = 0; i < 200; ++i) {
        result = session.pollValidation();
        if (result.state != logic::EditorValidationState::RUNNING)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    CHECK(result.state == logic::EditorValidationState::STALE);
    CHECK_FALSE(result.valid);
    CHECK(result.generation == generationBefore);
    CHECK(result.message ==
          "Resultado descartado: o documento foi alterado durante a validação");
}

} // TEST_SUITE