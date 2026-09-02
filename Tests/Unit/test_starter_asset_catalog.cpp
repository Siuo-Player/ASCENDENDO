#include <doctest/doctest.h>

#include "Assets/StarterAssetCatalog.h"

TEST_CASE("starter catalog exposes only verified runtime asset") {
    REQUIRE(assets::kStarterRuntimeAssets.size() == 1);

    const assets::AssetMetadata& player = assets::kStarterRuntimeAssets.front();
    CHECK(player.assetId == "player.personagem.v1");
    CHECK(player.runtimePath == "Game/Assets/Sprites/personagem.png");
    CHECK(player.sourcePath == "Game/Assets/Sprites/Source/personagem.pixil");
    CHECK(player.licence == "project-owned");
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
