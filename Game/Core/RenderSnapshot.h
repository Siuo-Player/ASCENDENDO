#pragma once
// =============================================================================
//  Game/Core/RenderSnapshot.h
//
//  Presentation data extracted from runtime state.
//  This contract deliberately contains no gameplay/domain types and no Vulkan
//  ownership. It is a transient value object consumed by rendering passes.
// =============================================================================

#include <vector>

namespace core {

struct RenderRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct RenderPlayer {
    RenderRect bounds{};
    bool facingLeft = false;
};

struct RenderFlag {
    bool visible = false;
    RenderRect bounds{};
};

struct RenderSnapshot {
    RenderPlayer player{};
    std::vector<RenderRect> platforms;
    RenderFlag flag{};
};

} // namespace core
