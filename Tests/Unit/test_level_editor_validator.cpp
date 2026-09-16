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

    // Each step is below the validator's effective maximum vertical jump.
    // The final implicit goal is the true top of the two-screen document.
    REQUIRE(document.addPlatform({{100.0f, 100.0f}, {200.0f, 116.0f}}));
    REQUIRE(document.addPlatform({{100.0f, 190.0f}, {200.0f, 206.0f}}));
    REQUIRE(document.addPlatform({{100.0f, 280.0f}, {200.0f, 296.0f}}));

    const EditorValidationResult result = validateEditorDocument(document);

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
    LevelEditorDocument document(true, ground, 1);

    // A -> P establishes the upward path. P reaches X and Y. X reaches the
    // highest platform B (and therefore the automatic goal), while Y reaches
    // C. X cannot reach C horizontally. The queue at that point is ordered
    // as Y, B, GOAL; Y discovers C behind GOAL. The old `break` at GOAL then
    // leaves C unprocessed and falsely marked unreachable.
    REQUIRE(document.addPlatform({{100.0f, 70.0f}, {180.0f, 86.0f}}));  // A
    REQUIRE(document.addPlatform({{100.0f, 150.0f}, {620.0f, 170.0f}})); // P
    REQUIRE(document.addPlatform({{100.0f, 250.0f}, {180.0f, 266.0f}})); // X
    REQUIRE(document.addPlatform({{600.0f, 220.0f}, {620.0f, 236.0f}})); // Y
    REQUIRE(document.addPlatform({{600.0f, 300.0f}, {620.0f, 316.0f}})); // C
    REQUIRE(document.addPlatform({{100.0f, 314.0f}, {180.0f, 330.0f}})); // B (goal)

    const EditorValidationResult result = validateEditorDocument(document);

    CHECK(result.reachesGoal);
    CHECK(result.valid);
    CHECK(result.reachablePlatforms == 5);
    REQUIRE(result.platformReachable.size() == 6);
    CHECK(result.platformReachable[0]);
    CHECK(result.platformReachable[1]);
    CHECK(result.platformReachable[2]);
    CHECK(result.platformReachable[3]);
    CHECK(result.platformReachable[4] == false);
    CHECK(result.platformReachable[5]);
}

} // TEST_SUITE
