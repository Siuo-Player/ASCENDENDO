#include "doctest/doctest.h"
#include "Logic/LevelDataValidator.h"

#include <limits>

using namespace logic;

TEST_SUITE("LevelDataValidator") {
    TEST_CASE("accepts valid platform, spawn and flag geometry") {
        LevelData data;
        data.spawnPosition = Vec2{120.0f, 40.0f};
        data.platforms.push_back({{10.0f, 20.0f}, {30.0f, 40.0f}});
        data.flag = AABB{{50.0f, 60.0f}, {70.0f, 100.0f}};

        CHECK(LevelDataValidator::validate(data));
    }

    TEST_CASE("accepts multi-screen geometry within total level bounds") {
        LevelData data;
        data.screenCount = 2;
        data.platforms.push_back({{10.0f, 350.0f}, {30.0f, 380.0f}});
        data.spawnPosition = Vec2{120.0f, 500.0f};

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

    TEST_CASE("rejects non-finite platform coordinate") {
        const float nan = std::numeric_limits<float>::quiet_NaN();
        LevelData data;
        data.platforms.push_back({{10.0f, 20.0f}, {nan, 40.0f}});

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects infinite flag coordinate") {
        const float inf = std::numeric_limits<float>::infinity();
        LevelData data;
        data.flag = AABB{{50.0f, 60.0f}, {70.0f, inf}};

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects non-finite lower coordinate") {
        const float negInf = -std::numeric_limits<float>::infinity();
        LevelData data;
        data.platforms.push_back({{negInf, 20.0f}, {30.0f, 40.0f}});

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects zero screen count") {
        LevelData data;
        data.screenCount = 0;
        data.platforms.push_back({{0.0f, 0.0f}, {16.0f, 16.0f}});

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects platform outside fixed logical width") {
        LevelData data;
        data.platforms.push_back({{-1.0f, 20.0f}, {30.0f, 40.0f}});
        CHECK_FALSE(LevelDataValidator::validate(data));

        data.platforms.clear();
        data.platforms.push_back({{600.0f, 20.0f}, {641.0f, 40.0f}});
        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects geometry beyond total vertical level bounds") {
        LevelData data;
        data.platforms.push_back({{10.0f, 350.0f}, {30.0f, 361.0f}});
        CHECK_FALSE(LevelDataValidator::validate(data));

        data.screenCount = 2;
        CHECK(LevelDataValidator::validate(data));

        data.platforms[0].max.y = 721.0f;
        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects authored spawn outside logical level bounds") {
        LevelData data;
        data.spawnPosition = Vec2{-0.1f, 40.0f};
        CHECK_FALSE(LevelDataValidator::validate(data));

        data.spawnPosition = Vec2{320.0f, 360.1f};
        CHECK_FALSE(LevelDataValidator::validate(data));

        data.screenCount = 2;
        data.spawnPosition = Vec2{320.0f, 600.0f};
        CHECK(LevelDataValidator::validate(data));
    }

    TEST_CASE("rejects an impossible total level height") {
        LevelData data;
        data.screenCount = std::numeric_limits<std::size_t>::max();

        CHECK_FALSE(LevelDataValidator::validate(data));
    }

    TEST_CASE("accepts level without optional flag") {
        LevelData data;
        data.platforms.push_back({{0.0f, 0.0f}, {640.0f, 20.0f}});

        CHECK(LevelDataValidator::validate(data));
    }
}
