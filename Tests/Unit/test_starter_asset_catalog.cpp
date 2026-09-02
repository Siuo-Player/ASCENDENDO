#include <string>

#include <doctest/doctest.h>

#include "Assets/StarterAssetCatalog.h"

// Materialize string_view metadata before doctest comparison so the assertions
// remain portable across the Linux and Windows toolchains used by CI.
// Keep the catalogue itself string_view-based; this conversion is test-only.

TEST_CASE("starter catalog exposes only verified runtime asset") {
    REQUIRE(assets::kStarterRuntimeAssets.size() == 1);

    const assets::AssetMetadata& player = assets::kStarterRuntimeAssets.front();
    const std::string assetId(player.assetId);
    const std::string runtimePath(player.runtimePath);
    const std::string sourcePath(player.sourcePath);
    const std::string licence(player.licence);

    CHECK(assetId == "player.personagem.v1");
    CHECK(runtimePath == "Game/Assets/Sprites/personagem.png");
    CHECK(sourcePath == "Game/Assets/Sprites/Source/personagem.pixil");
    CHECK(licence == "project-owned");
    CHECK(player.semanticRole == assets::SemanticRole::Player);
    CHECK(player.nominalPixelSize.width == 32);
    CHECK(player.nominalPixelSize.height == 32);
    CHECK(player.logicalDrawSize.width == 16);
    CHECK(player.logicalDrawSize.height == 16);
    CHECK(player.gameplayCritical);
    CHECK(player.availability == assets::Availability::Runtime);
}

TEST_CASE("starter catalog keeps missing visual roles explicitly reserved") {
    CHECK(assets::kReservedStarterRoles.size() == 13);

    for (const assets::SemanticRole role : assets::kReservedStarterRoles) {
        CHECK(assets::findRuntimeAsset(role) == nullptr);
    }

    CHECK(assets::findRuntimeAsset(assets::SemanticRole::Player) != nullptr);
}

TEST_CASE("starter player metadata preserves presentation-only contract") {
    const assets::AssetMetadata* player =
        assets::findRuntimeAsset(assets::SemanticRole::Player);
    REQUIRE(player != nullptr);

    CHECK(player->logicalDrawSize.width == 16);
    CHECK(player->logicalDrawSize.height == 16);
    CHECK(player->anchor.x == 0);
    CHECK(player->anchor.y == 0);
    CHECK_FALSE(player->flipXAllowed);
}
