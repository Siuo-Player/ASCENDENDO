#include "Graphics/StarterVisualCatalog.h"

#include <utility>

namespace gfx {

StarterVisualCatalog::StarterVisualCatalog(std::filesystem::path playerSpritePath)
    : player_{
          "player.character.personagem.v1",
          std::move(playerSpritePath),
          "Game/Assets/Sprites/Source/personagem.pixil",
          "UNVERIFIED",
          "local-personagem-pixil:297c4314194cede7c1d0698ec274a510410fcd67",
          StarterAssetRole::Player,
          32,
          32,
          16.0f,
          16.0f,
          StarterAssetAnchor::BottomLeft,
          true,
          true,
          0.0f,
          StarterAssetProvenance::Unverified} {}

} // namespace gfx
