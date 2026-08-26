#pragma once

#include "TextPipeline.h"
#include "FontRenderer.h"
#include "SpritePipeline.h"
#include "SpriteRenderer.h"

#include <filesystem>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;
class RendererFacade;

class PresentationRuntime {
public:
    PresentationRuntime() = default;
    ~PresentationRuntime();

    PresentationRuntime(const PresentationRuntime&) = delete;
    PresentationRuntime& operator=(const PresentationRuntime&) = delete;
    PresentationRuntime(PresentationRuntime&&) = delete;
    PresentationRuntime& operator=(PresentationRuntime&&) = delete;

    bool init(VulkanContext* ctx,
              Swapchain* swapchain,
              RenderPass* renderPass,
              RendererFacade* renderer,
              const std::filesystem::path& playerSpritePath);

    bool textReady() const { return textReady_; }
    bool spriteReady() const { return spriteReady_; }
    bool anyReady() const { return textReady_ || spriteReady_; }

    TextPipeline& textPipeline() { return textPipeline_; }
    FontRenderer& font() { return font_; }
    SpritePipeline& spritePipeline() { return spritePipeline_; }
    SpriteRenderer& playerSprite() { return playerSprite_; }

private:
    TextPipeline textPipeline_;
    FontRenderer font_;
    SpritePipeline spritePipeline_;
    SpriteRenderer playerSprite_;
    RendererFacade* renderer_ = nullptr;
    bool textReady_ = false;
    bool spriteReady_ = false;
};

} // namespace gfx
