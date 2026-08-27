#pragma once

#include "Window.h"
#include "VulkanContext.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "RendererFacade.h"

namespace gfx {

// Owns the core graphics bootstrap in dependency/destruction order.
// Optional text/sprite pipelines remain outside this boundary for now.
class GraphicsRuntime {
public:
    GraphicsRuntime() = default;
    ~GraphicsRuntime() { cleanup(); }

    GraphicsRuntime(const GraphicsRuntime&) = delete;
    GraphicsRuntime& operator=(const GraphicsRuntime&) = delete;
    GraphicsRuntime(GraphicsRuntime&&) = delete;
    GraphicsRuntime& operator=(GraphicsRuntime&&) = delete;

    bool init(uint32_t width, uint32_t height, const char* title);

    Window& window() { return window_; }
    const Window& window() const { return window_; }
    VulkanContext& context() { return context_; }
    const VulkanContext& context() const { return context_; }
    Swapchain& swapchain() { return swapchain_; }
    const Swapchain& swapchain() const { return swapchain_; }
    RenderPass& renderPass() { return renderPass_; }
    const RenderPass& renderPass() const { return renderPass_; }
    Pipeline& pipeline() { return pipeline_; }
    const Pipeline& pipeline() const { return pipeline_; }
    RendererFacade& renderer() { return renderer_; }
    const RendererFacade& renderer() const { return renderer_; }

    bool isInitialized() const;

private:
    void cleanup();

    Window window_;
    VulkanContext context_;
    Swapchain swapchain_;
    RenderPass renderPass_;
    Pipeline pipeline_;
    RendererFacade renderer_;
};

} // namespace gfx
