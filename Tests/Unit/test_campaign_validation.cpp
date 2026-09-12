#include "doctest/doctest.h"
#include "Core/CampaignLoader.h"
#include "Logic/CampaignValidation.h"

#include <string>
#include <vector>

TEST_SUITE("Campaign validation") {
    TEST_CASE("admin validator matches active campaign physics contract") {
        const auto paths = core::CampaignLoader::load(
            "Game/Assets/Levels/campaign.txt",
            "Game/Assets/Levels");
        REQUIRE(paths.size() == 25);

        std::vector<std::string> stringPaths;
        stringPaths.reserve(paths.size());
        for (const auto& path : paths) stringPaths.push_back(path.string());

        const auto report = logic::validateCampaignForAdmin(stringPaths);
        CHECK(report.entries.size() == 25);
        CHECK(report.validLevels == 25);
        CHECK(report.allValid());
        for (const auto& entry : report.entries) {
            CHECK(entry.valid);
            CHECK(entry.reachablePlatforms == entry.totalPlatforms);
        }
    }
}
