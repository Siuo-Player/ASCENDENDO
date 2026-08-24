// =============================================================================
// Game/Graphics/RendererCore.cpp
//
// Novo núcleo de frame Vulkan. Este ficheiro não conhece Player, Level, menus
// ou editor; apenas possui recursos/frame lifecycle e submit/present.
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

    if (!createFramebuffers()) return false;
    if (!createCommandPool()) return false;
    if (!allocateCommandBuffers()) return false;
    if (!createSyncObjects()) return false;

    m_initialized = true;
    return true;
}

void RendererCore::cleanup() {
    if (!m_initialized) return;

    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);

    for (VkFramebuffer framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    m_framebuffers.clear();

    if (m_inFlightFence) {
        vkDestroyFence(device, m_inFlightFence, nullptr);
    }
    if (m_renderFinishedSemaphore) {
        vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
    }
    if (m_imageAvailableSemaphore) {
        vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
    }
    if (m_commandPool) {
        vkDestroyCommandPool(device, m_commandPool, nullptr);
    }

    m_commandBuffers.clear();
    m_inFlightFence = VK_NULL_HANDLE;
    m_renderFinishedSemaphore = VK_NULL_HANDLE;
    m_imageAvailableSemaphore = VK_NULL_HANDLE;
    m_commandPool = VK_NULL_HANDLE;
    m_ctx = nullptr;
    m_swapchain = nullptr;
    m_renderPass = nullptr;
    m_pipeline = nullptr;
    m_initialized = false;
}

bool RendererCore::beginFrame(VkCommandBuffer& commandBuffer, uint32_t& imageIndex) {
    if (!m_initialized) return false;

    VkDevice device = m_ctx->device();
    if (vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        return false;
    }

    VkResult result = vkAcquireNextImageKHR(
        device,
        m_swapchain->handle(),
        UINT64_MAX,
        m_imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return false;
    }

    if (vkResetFences(device, 1, &m_inFlightFence) != VK_SUCCESS) {
        return false;
    }
    if (vkResetCommandBuffer(m_commandBuffers[imageIndex], 0) != VK_SUCCESS) {
        return false;
    }

    commandBuffer = m_commandBuffers[imageIndex];
    return true;
}

bool RendererCore::submitFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    if (!m_initialized) return false;

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
        return false;
    }

    VkSwapchainKHR swapchain = m_swapchain->handle();
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(m_ctx->graphicsQueue(), &presentInfo);
    return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
}

VkCommandBuffer RendererCore::commandBuffer(uint32_t imageIndex) const {
    if (imageIndex >= m_commandBuffers.size()) return VK_NULL_HANDLE;
    return m_commandBuffers[imageIndex];
}

VkExtent2D RendererCore::swapchainExtent() const {
    return m_swapchain ? m_swapchain->extent() : VkExtent2D{};
}

bool RendererCore::createFramebuffers() {
    m_framebuffers.resize(m_swapchain->imageViews().size());
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
            return false;
        }
    }
    return true;
}

bool RendererCore::createCommandPool() {
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = m_ctx->graphicsFamily();
    return vkCreateCommandPool(m_ctx->device(), &info, nullptr, &m_commandPool) == VK_SUCCESS;
}

bool RendererCore::allocateCommandBuffers() {
    m_commandBuffers.resize(m_swapchain->imageViews().size());

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_commandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    return vkAllocateCommandBuffers(
               m_ctx->device(), &info, m_commandBuffers.data()) == VK_SUCCESS;
}

bool RendererCore::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = m_ctx->device();
    return vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) == VK_SUCCESS
        && vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) == VK_SUCCESS
        && vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFence) == VK_SUCCESS;
}

} // namespace gfx
