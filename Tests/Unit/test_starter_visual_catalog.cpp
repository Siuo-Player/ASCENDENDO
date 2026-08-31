#include "doctest/doctest.h"
#include "Graphics/StarterVisualCatalog.h"

TEST_SUITE("starter visual catalog") {

    TEST_CASE("current player entry preserves staged asset facts") {
        const gfx::StarterVisualCatalog catalog("Game/Assets/Sprites/personagem.png");
        const auto& player = catalog.player();

        CHECK(player.assetId == "player.character.personagem.v1");
        CHECK(player.runtimePath == "Game/Assets/Sprites/personagem.png");
        CHECK(player.sourcePath == "Game/Assets/Sprites/Source/personagem.pixil");
        CHECK(player.licence == "UNVERIFIED");
        CHECK(player.provenanceId ==
              "local-personagem-pixil:297c4314194cede7c1d0698ec274a510410fcd67");
        CHECK(player.semanticRole == gfx::StarterAssetRole::Player);
        CHECK(player.nominalPixelWidth == 32);
        CHECK(player.nominalPixelHeight == 32);
        CHECK(player.logicalDrawWidth == doctest::Approx(16.0f));
        CHECK(player.logicalDrawHeight == doctest::Approx(16.0f));
        CHECK(player.anchor == gfx::StarterAssetAnchor::BottomLeft);
        CHECK(player.flipAllowed);
        CHECK(player.gameplayCritical);
        CHECK(player.visualContactEdgeFromBottom == doctest::Approx(0.0f));
        CHECK(player.provenance == gfx::StarterAssetProvenance::Unverified);
    }

    TEST_CASE("catalog does not silently turn missing licence provenance into approval") {
        const gfx::StarterVisualCatalog catalog("custom/player.png");
        const auto& player = catalog.player();

        CHECK(player.provenance != gfx::StarterAssetProvenance::Verified);
        CHECK(player.licence == "UNVERIFIED");
    }
}
