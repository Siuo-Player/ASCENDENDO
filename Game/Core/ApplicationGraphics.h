#pragma once

struct GLFWwindow;

namespace gfx {
class Window;
class VulkanContext;
class Swapchain;
class RenderPass;
class Pipeline;
class TextPipeline;
class FontRenderer;
class SpritePipeline;
class SpriteRenderer;
class RendererFacade;
}

namespace app {

bool initializeGraphics(
    gfx::Window& window,
    gfx::VulkanContext& context,
    gfx::Swapchain& swapchain,
    gfx::RenderPass& renderPass,
    gfx::Pipeline& pipeline,
    gfx::TextPipeline& textPipeline,
    gfx::FontRenderer& font,
    gfx::SpritePipeline& spritePipeline,
    gfx::SpriteRenderer& playerSprite,
    gfx::RendererFacade& renderer,
    int screenWidth,
    int screenHeight);

} // namespace app
