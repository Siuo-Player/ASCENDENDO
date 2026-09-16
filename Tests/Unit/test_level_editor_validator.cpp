// =============================================================================
// Tests/Unit/test_level_editor_validator.cpp
// =============================================================================
// Regression coverage for the native editor reachability graph.

#include "doctest/doctest.h"
#include "Logic/LevelEditor.h"
#include "Logic/LevelEditorValidator.h"

using namespace logic;

TEST_SUITE("LevelEditorValidator — multi-screen reachability") {

TEST_CASE("goal implicito usa a altura total do documento, nao uma screen") {
    const AABB ground{{0.0f, 0.0f}, {640.0f, 16.0f}};
    LevelEditorDocument document(false, ground, 2);

    REQUIRE(document.levelHeight() == doctest::Approx(720.0f));

    REQUIRE(document.addPlatform({{100.0f, 120.0f}, {200.0f, 136.0f}}));
    REQUIRE(document.addPlatform({{100.0f, 230.0f}, {200.0f, 246.0f}}));
    REQUIRE(document.addPlatform({{100.0f, 340.0f}, {200.0f, 356.0f}}));

    const EditorValidationResult result = validateEditorDocument(document);

    // The document has no authored flag because this is not the final
    // campaign level. Its implicit goal is the top of the whole 2-screen
    // document, so the three reached platforms are not enough to finish.
    CHECK(result.reachesGoal == false);
    CHECK(result.valid == false);
    CHECK(result.reachablePlatforms == 3);
    REQUIRE(result.platformReachable.size() == 3);
    CHECK(result.platformReachable[0]);
    CHECK(result.platformReachable[1]);
    CHECK(result.platformReachable[2]);
}

TEST_CASE("BFS continua depois de atingir o objetivo para marcar todos os nos alcancaveis") {
    const AABB ground{{0.0f, 0.0f}, {640.0f, 16.0f}};
    LevelEditorDocument document(false, ground, 1);

    // A is reachable from ground and can reach the implicit goal. C is also
    // reachable from ground, and C can reach B. A cannot reach B horizontally.
    // Therefore B is enqueued only after the goal, exposing the premature
    // `break` that used to leave B marked unreachable.
    REQUIRE(document.addPlatform({{20.0f, 230.0f}, {60.0f, 246.0f}}));   // A
    REQUIRE(document.addPlatform({{400.0f, 120.0f}, {440.0f, 136.0f}})); // C
    REQUIRE(document.addPlatform({{400.0f, 230.0f}, {440.0f, 246.0f}})); // B

    const EditorValidationResult result = validateEditorDocument(document);

    CHECK(result.reachesGoal);
    CHECK(result.valid);
    CHECK(result.reachablePlatforms == 3);
    REQUIRE(result.platformReachable.size() == 3);
    CHECK(result.platformReachable[0]);
    CHECK(result.platformReachable[1]);
    CHECK(result.platformReachable[2]);
}

} // TEST_SUITE
