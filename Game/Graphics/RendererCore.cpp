// =============================================================================
// Game/Graphics/RendererCore.cpp
// =============================================================================
#include "Graphics/RendererCore.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Pipeline.h"

namespace gfx {

bool RendererCore::init(VulkanContext* ctx, Swapchain* swapchain,
                        RenderPass* renderPass, Pipeline* pipeline) {
    if (m_initialized) return true;
    if (!ctx || !swapchain || !renderPass || !pipeline) return false;
    if (!ctx->isInitialized() || !swapchain->isInitialized() ||
        !renderPass->isInitialized() || !pipeline->isInitialized()) return false;

    m_ctx = ctx;
    m_swapchain = swapchain;
    m_renderPass = renderPass;
    m_pipeline = pipeline;

    if (!createFramebuffers() || !createCommandPool() ||
        !allocateCommandBuffers() || !createSyncObjects()) {
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void RendererCore::destroyFrameResources() {
    if (!m_ctx) return;
    VkDevice device = m_ctx->device();

    for (VkFramebuffer framebuffer : m_framebuffers) {
        if (framebuffer) vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    m_framebuffers.clear();

    if (m_commandPool) {
        vkDestroyCommandPool(device, m_commandPool, nullptr);
    }
    m_commandPool = VK_NULL_HANDLE;
    m_commandBuffers.clear();

    if (m_inFlightFence) vkDestroyFence(device, m_inFlightFence, nullptr);
    if (m_renderFinishedSemaphore) {
        vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
    }
    if (m_imageAvailableSemaphore) {
        vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
    }

    m_inFlightFence = VK_NULL_HANDLE;
    m_renderFinishedSemaphore = VK_NULL_HANDLE;
    m_imageAvailableSemaphore = VK_NULL_HANDLE;
}

void RendererCore::cleanup() {
    if (!m_ctx) return;

    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);
    destroyFrameResources();

    m_ctx = nullptr;
    m_swapchain = nullptr;
    m_renderPass = nullptr;
    m_pipeline = nullptr;
    m_initialized = false;
}

RendererCore::FrameStatus RendererCore::beginFrame(VkCommandBuffer& commandBuffer,
                                                     uint32_t& imageIndex) {
    if (!m_initialized || !m_ctx || !m_swapchain) return FrameStatus::Fatal;

    VkDevice device = m_ctx->device();
    if (vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        return FrameStatus::Fatal;
    }

    const VkResult result = vkAcquireNextImageKHR(
        device, m_swapchain->handle(), UINT64_MAX,
        m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return FrameStatus::SwapchainNeedsRecreate;
    }
    if (result != VK_SUCCESS) return FrameStatus::Fatal;

    if (imageIndex >= m_commandBuffers.size()) return FrameStatus::Fatal;
    if (vkResetCommandBuffer(m_commandBuffers[imageIndex], 0) != VK_SUCCESS) {
        return FrameStatus::Fatal;
    }

    commandBuffer = m_commandBuffers[imageIndex];
    return FrameStatus::Ready;
}

bool RendererCore::beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                                   float clearR, float clearG, float clearB) {
    if (!m_initialized || commandBuffer == VK_NULL_HANDLE ||
        imageIndex >= m_framebuffers.size()) return false;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) return false;

    VkClearValue clearValue{};
    clearValue.color = {{clearR, clearG, clearB, 1.0f}};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass->handle();
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchain->extent();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    return true;
}

bool RendererCore::endRenderPass(VkCommandBuffer commandBuffer) {
    if (!m_initialized || commandBuffer == VK_NULL_HANDLE) return false;
    vkCmdEndRenderPass(commandBuffer);
    return vkEndCommandBuffer(commandBuffer) == VK_SUCCESS;
}

RendererCore::FrameStatus RendererCore::submitFrame(VkCommandBuffer commandBuffer,
                                                      uint32_t imageIndex) {
    if (!m_initialized || commandBuffer == VK_NULL_HANDLE ||
        imageIndex >= m_commandBuffers.size()) return FrameStatus::Fatal;

    VkDevice device = m_ctx->device();

    // Reset immediately before submission. If command recording fails before
    // this point, the signaled fence remains usable by the next frame.
    if (vkResetFences(device, 1, &m_inFlightFence) != VK_SUCCESS) {
        return FrameStatus::Fatal;
    }

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;

    if (vkQueueSubmit(m_ctx->graphicsQueue(), 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        // Submission failure is not safely recoverable for this frame because
        // the fence has already been reset. Let the application fail closed
        // instead of attempting to reuse partially submitted synchronization.
        return FrameStatus::Fatal;
    }

    VkSwapchainKHR swapchain = m_swapchain->handle();
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    const VkResult result = vkQueuePresentKHR(m_ctx->presentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return FrameStatus::SwapchainNeedsRecreate;
    }
    if (result != VK_SUCCESS) return FrameStatus::Fatal;

    return FrameStatus::Ready;
}

bool RendererCore::recreateSwapchain() {
    if (!m_initialized || !m_ctx || !m_swapchain) return false;

    VkDevice device = m_ctx->device();
    if (m_waitIdle(device) != VK_SUCCESS) return false;

    // From this point onward the old frame resources are deliberately gone.
    // Mark the core inactive before destroying anything so a failed rebuild
    // can never leave a half-valid object that drawFrame() might reuse.
    m_initialized = false;
    destroyFrameResources();

    if (!m_swapchain->recreate()) {
        return false;
    }

    if (!createFramebuffers() || !createCommandPool() ||
        !allocateCommandBuffers() || !createSyncObjects()) {
        destroyFrameResources();
        return false;
    }

    m_initialized = true;
    return true;
}

VkExtent2D RendererCore::swapchainExtent() const {
    return m_swapchain ? m_swapchain->extent() : VkExtent2D{};
}

bool RendererCore::createFramebuffers() {
    if (!m_swapchain || !m_ctx || !m_renderPass) return false;

    m_framebuffers.assign(m_swapchain->imageViews().size(), VK_NULL_HANDLE);
    for (size_t i = 0; i < m_framebuffers.size(); ++i) {
        VkImageView attachments[] = {m_swapchain->imageViews()[i]};
        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_renderPass->handle();
        info.attachmentCount = 1;
        info.pAttachments = attachments;
        info.width = m_swapchain->extent().width;
        info.height = m_swapchain->extent().height;
        info.layers = 1;

        if (vkCreateFramebuffer(m_ctx->device(), &info, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            for (auto framebuffer : m_framebuffers) {
                if (framebuffer) vkDestroyFramebuffer(m_ctx->device(), framebuffer, nullptr);
            }
            m_framebuffers.clear();
            return false;
        }
    }
    return true;
}

bool RendererCore::createCommandPool() {
    if (!m_ctx) return false;
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = m_ctx->graphicsFamily();
    return vkCreateCommandPool(m_ctx->device(), &info, nullptr, &m_commandPool) == VK_SUCCESS;
}

bool RendererCore::allocateCommandBuffers() {
    if (!m_ctx || !m_swapchain || !m_commandPool) return false;

    m_commandBuffers.assign(m_swapchain->imageViews().size(), VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_commandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    return vkAllocateCommandBuffers(m_ctx->device(), &info, m_commandBuffers.data()) == VK_SUCCESS;
}

bool RendererCore::createSyncObjects() {
    if (!m_ctx) return false;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = m_ctx->device();
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS) {
        return false;
    }
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS) {
        vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
        return false;
    }
    if (vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) {
        vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

} // namespace gfx
