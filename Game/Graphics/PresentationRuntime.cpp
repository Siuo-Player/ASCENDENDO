#include "PresentationRuntime.h"

#include "VulkanContext.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "RendererFacade.h"
#include "Graphics/StarterVisualCatalog.h"

namespace gfx {

PresentationRuntime::~PresentationRuntime() {
    if (renderer_) {
        renderer_->attachText(nullptr, nullptr);
        renderer_->attachSprite(nullptr, nullptr);
        renderer_ = nullptr;
    }
}

bool PresentationRuntime::init(VulkanContext* ctx,
                               Swapchain* swapchain,
                               RenderPass* renderPass,
                               RendererFacade* renderer,
                               const std::filesystem::path& playerSpritePath) {
    if (!ctx || !swapchain || !renderPass || !renderer ||
        !ctx->isInitialized() || !swapchain->isInitialized() ||
        !renderPass->isInitialized() || !renderer->isInitialized()) {
        return false;
    }

    renderer_ = renderer;
    textReady_ = false;
    spriteReady_ = false;

    if (textPipeline_.init(ctx, swapchain, renderPass) &&
        font_.init(ctx, textPipeline_.descriptorSetLayout())) {
        renderer_->attachText(&textPipeline_, &font_);
        textReady_ = true;
    } else {
        textPipeline_.cleanup();
        font_.cleanup();
    }

    const StarterVisualCatalog starterAssets(playerSpritePath);
    const StarterAssetMetadata& playerAsset = starterAssets.player();
    if (spritePipeline_.init(ctx, swapchain, renderPass) &&
        playerSprite_.init(ctx, spritePipeline_.descriptorSetLayout(), playerAsset.runtimePath.string())) {
        renderer_->attachSprite(&spritePipeline_, &playerSprite_);
        spriteReady_ = true;
    } else {
        spritePipeline_.cleanup();
        playerSprite_.cleanup();
    }

    return true;
}

} // namespace gfx
