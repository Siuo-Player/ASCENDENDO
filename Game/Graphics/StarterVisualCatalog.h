#pragma once

#include <filesystem>
#include <string>

namespace gfx {

enum class StarterAssetRole {
    Player,
};

enum class StarterAssetAnchor {
    BottomLeft,
};

enum class StarterAssetProvenance {
    Verified,
    Unverified,
};

struct StarterAssetMetadata {
    std::string assetId;
    std::filesystem::path runtimePath;
    std::string sourcePath;
    std::string licence;
    std::string provenanceId;
    StarterAssetRole semanticRole;
    int nominalPixelWidth = 0;
    int nominalPixelHeight = 0;
    float logicalDrawWidth = 0.0f;
    float logicalDrawHeight = 0.0f;
    StarterAssetAnchor anchor = StarterAssetAnchor::BottomLeft;
    bool flipAllowed = false;
    bool gameplayCritical = false;
    float visualContactEdgeFromBottom = 0.0f;
    StarterAssetProvenance provenance = StarterAssetProvenance::Unverified;
};

class StarterVisualCatalog {
public:
    explicit StarterVisualCatalog(std::filesystem::path playerSpritePath);

    const StarterAssetMetadata& player() const noexcept { return player_; }

private:
    StarterAssetMetadata player_;
};

} // namespace gfx
