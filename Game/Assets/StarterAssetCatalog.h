#pragma once

#include <array>
#include <string_view>

namespace assets {

enum class SemanticRole {
    Player,
    PlatformStraight,
    PlatformEndLeft,
    PlatformEndRight,
    PlatformCornerLeft,
    PlatformCornerRight,
    Hazard,
    CheckpointInactive,
    CheckpointActive,
    Goal,
    RecoveryPlatform,
    SupportProp,
    Landmark,
    Atmosphere,
};

enum class Availability {
    Runtime,
    Reserved,
};

struct PixelSize {
    int width = 0;
    int height = 0;
};

struct AssetMetadata {
    std::string_view assetId;
    std::string_view runtimePath;
    std::string_view sourcePath;
    std::string_view licence;
    std::string_view provenanceId;
    SemanticRole semanticRole;
    PixelSize nominalPixelSize;
    PixelSize logicalDrawSize;
    PixelSize anchor;
    bool flipXAllowed = false;
    bool gameplayCritical = false;
    Availability availability = Availability::Reserved;
};

// This is intentionally a tiny semantic catalogue. External candidates are
// not represented as runtime assets until provenance and visual review pass.
inline constexpr std::array<AssetMetadata, 1> kStarterRuntimeAssets{{
    {
        "player.personagem.v1",
        "Game/Assets/Sprites/personagem.png",
        "Game/Assets/Sprites/Source/personagem.pixil",
        "project-owned",
        "ascendendo.personagem.v1",
        SemanticRole::Player,
        {32, 32},
        {16, 16},
        {0, 0},
        false,
        true,
        Availability::Runtime,
    },
}};

inline constexpr std::array<SemanticRole, 13> kReservedStarterRoles{{
    SemanticRole::PlatformStraight,
    SemanticRole::PlatformEndLeft,
    SemanticRole::PlatformEndRight,
    SemanticRole::PlatformCornerLeft,
    SemanticRole::PlatformCornerRight,
    SemanticRole::Hazard,
    SemanticRole::CheckpointInactive,
    SemanticRole::CheckpointActive,
    SemanticRole::Goal,
    SemanticRole::RecoveryPlatform,
    SemanticRole::SupportProp,
    SemanticRole::Landmark,
    SemanticRole::Atmosphere,
}};

constexpr const AssetMetadata* findRuntimeAsset(SemanticRole role) {
    for (const AssetMetadata& asset : kStarterRuntimeAssets) {
        if (asset.semanticRole == role) {
            return &asset;
        }
    }
    return nullptr;
}

} // namespace assets
