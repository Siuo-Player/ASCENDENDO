#include "doctest/doctest.h"
#include "Logic/LevelDataValidator.h"

using namespace logic;

TEST_SUITE("LevelDataValidator") {
    TEST_CASE("accepts valid platform and flag geometry") {
        LevelData data;
        data.platforms.push_back({{10.0f, 20.0f}, {30.0f, 40.0f}});
        data.flag = AABB{{50.0f, 60.0f}, {70.0f, 100.0f}};

        CHECK(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects zero-width platform") {
        LevelData data;
        data.platforms.push_back({{10.0f, 20.0f}, {10.0f, 40.0f}});

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects negative-height platform") {
        LevelData data;
        data.platforms.push_back({{10.0f, 40.0f}, {30.0f, 20.0f}});

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects zero-height flag") {
        LevelData data;
        data.flag = AABB{{50.0f, 60.0f}, {70.0f, 60.0f}};

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("accepts level without optional flag") {
        LevelData data;
        data.platforms.push_back({{0.0f, 0.0f}, {640.0f, 20.0f}});

        CHECK(LevelDataValidator::validate(data));
    }
}
