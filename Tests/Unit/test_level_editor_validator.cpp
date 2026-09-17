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

} // TEST_SUITE
