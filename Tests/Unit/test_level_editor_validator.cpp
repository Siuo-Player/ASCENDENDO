#include "doctest/doctest.h"
#include "Logic/LevelEditor.h"
#include "Logic/LevelEditorValidator.h"

#include <algorithm>

namespace {

logic::AABB platform(float x, float y, float width = 96.0f, float height = 16.0f) {
    return {{x, y}, {x + width, y + height}};
}

void addVerticalChain(logic::LevelEditorDocument& document,
                      int count,
                      float firstY = 80.0f,
                      float spacing = 90.0f) {
    for (int i = 0; i < count; ++i) {
        REQUIRE(document.addPlatform(platform(272.0f,
                                               firstY + spacing * static_cast<float>(i))));
    }
}

} // namespace

TEST_SUITE("Level editor reachability validation") {
    TEST_CASE("validates a complete one-screen platform chain") {
        logic::LevelEditorDocument document(false, {
            {0.0f, 0.0f}, {640.0f, 16.0f}
        }, 1);
        addVerticalChain(document, 4, 64.0f, 70.0f);

        const auto result = logic::validateEditorDocument(document);

        CHECK(result.valid);
        CHECK(result.reachesGoal);
        CHECK(result.totalPlatforms == 4);
        CHECK(result.reachablePlatforms == 4);
        CHECK(std::all_of(result.platformReachable.begin(),
                          result.platformReachable.end(),
                          [](bool reachable) { return reachable; }));
    }

    TEST_CASE("uses the real multi-screen height for the implicit goal") {
        logic::LevelEditorDocument document(false, {
            {0.0f, 0.0f}, {640.0f, 16.0f}
        }, 3);
        addVerticalChain(document, 11, 40.0f, 90.0f);

        const auto incomplete = logic::validateEditorDocument(document);
        CHECK_FALSE(incomplete.valid);
        CHECK_FALSE(incomplete.reachesGoal);
        CHECK(incomplete.reachablePlatforms == 11);
        CHECK(incomplete.totalPlatforms == 11);

        REQUIRE(document.addPlatform(platform(272.0f, 1030.0f)));
        const auto complete = logic::validateEditorDocument(document);
        CHECK(complete.valid);
        CHECK(complete.reachesGoal);
        CHECK(complete.reachablePlatforms == 12);
        CHECK(complete.totalPlatforms == 12);
    }

    TEST_CASE("final campaign goal follows the highest platform automatically") {
        logic::LevelEditorDocument document(true, {
            {0.0f, 0.0f}, {640.0f, 16.0f}
        }, 3);
        addVerticalChain(document, 11, 40.0f, 90.0f);

        REQUIRE(document.hasFlag());
        const auto firstGoal = *document.flag();
        CHECK(firstGoal.min.y == doctest::Approx(956.0f));

        const auto firstResult = logic::validateEditorDocument(document);
        CHECK(firstResult.valid);
        CHECK(firstResult.reachesGoal);
        CHECK(firstResult.reachablePlatforms == 11);

        REQUIRE(document.addPlatform(platform(272.0f, 1030.0f)));
        REQUIRE(document.hasFlag());
        CHECK(document.flag()->min.y == doctest::Approx(1046.0f));

        const auto secondResult = logic::validateEditorDocument(document);
        CHECK(secondResult.valid);
        CHECK(secondResult.reachesGoal);
        CHECK(secondResult.reachablePlatforms == 12);
    }
}
