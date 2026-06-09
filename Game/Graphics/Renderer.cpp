// =============================================================================
//  Game/Graphics/Renderer.cpp
//
//  @version 4.2
// =============================================================================
#include "Graphics/Renderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Camera.h"

namespace gfx {

bool Renderer::init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass) {
    if (m_initialized) return true;
    if (!ctx || !swapchain || !renderPass) return false;
    if (!ctx->isInitialized() || !swapchain->isInitialized() || !renderPass->isInitialized()) return false;

    m_ctx        = ctx;
    m_swapchain  = swapchain;
    m_renderPass = renderPass;

    if (!createFramebuffers())     return false;
    if (!createCommandPool())      return false;
    if (!allocateCommandBuffers()) return false;
    if (!createSyncObjects())      return false;

    m_initialized = true;
    return true;
}

void Renderer::cleanup() {
    if (!m_initialized) return;
    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);

    for (auto fb : m_framebuffers) {
        vkDestroyFramebuffer(device, fb, nullptr);
    }
    m_framebuffers.clear();

    if (m_inFlightFence)           vkDestroyFence(device, m_inFlightFence, nullptr);
    if (m_renderFinishedSemaphore) vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
    if (m_imageAvailableSemaphore) vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);

    if (m_commandPool) vkDestroyCommandPool(device, m_commandPool, nullptr);
    
    m_commandBuffers.clear();
    m_inFlightFence        = VK_NULL_HANDLE;
    m_commandPool          = VK_NULL_HANDLE;

    m_ctx = nullptr; m_swapchain = nullptr; m_renderPass = nullptr;
    m_initialized = false;
}

bool Renderer::drawFrame(float r, float g, float b) {
    VkDevice device = m_ctx->device();
    vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device, m_swapchain->handle(), UINT64_MAX,
                                            m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return false;

    vkResetFences(device, 1, &m_inFlightFence);
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);

    if (!recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex, r, g, b)) return false;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &m_imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &m_renderFinishedSemaphore;

    if (vkQueueSubmit(m_ctx->graphicsQueue(), 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) return false;

    VkSwapchainKHR sc = m_swapchain->handle();
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &sc;
    presentInfo.pImageIndices      = &imageIndex;

    result = vkQueuePresentKHR(m_ctx->graphicsQueue(), &presentInfo);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return false;

    return true;
}

// ── Inicialização e Comandos ──────────────────────────────────────────────────

bool Renderer::createFramebuffers() {
    m_framebuffers.resize(m_swapchain->imageViews().size());
    for (size_t i = 0; i < m_swapchain->imageViews().size(); i++) {
        VkImageView attachments[] = { m_swapchain->imageViews()[i] };

        VkFramebufferCreateInfo fbCI{};
        fbCI.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCI.renderPass      = m_renderPass->handle();
        fbCI.attachmentCount = 1;
        fbCI.pAttachments    = attachments;
        fbCI.width           = m_swapchain->extent().width;
        fbCI.height          = m_swapchain->extent().height;
        fbCI.layers          = 1;

        if (vkCreateFramebuffer(m_ctx->device(), &fbCI, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            return false;
        }
    }
    return true;
}

bool Renderer::createCommandPool() {
    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.queueFamilyIndex = m_ctx->graphicsFamily();
    return vkCreateCommandPool(m_ctx->device(), &poolCI, nullptr, &m_commandPool) == VK_SUCCESS;
}

bool Renderer::allocateCommandBuffers() {
    m_commandBuffers.resize(m_swapchain->imageViews().size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
    return vkAllocateCommandBuffers(m_ctx->device(), &allocInfo, m_commandBuffers.data()) == VK_SUCCESS;
}

bool Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo semCI{};
    semCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice dev = m_ctx->device();
    return vkCreateSemaphore(dev, &semCI, nullptr, &m_imageAvailableSemaphore) == VK_SUCCESS
        && vkCreateSemaphore(dev, &semCI, nullptr, &m_renderFinishedSemaphore) == VK_SUCCESS
        && vkCreateFence(dev, &fenceCI, nullptr, &m_inFlightFence)             == VK_SUCCESS;
}

bool Renderer::recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex, float r, float g, float b) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) return false;

    VkClearValue clearColor{};
    clearColor.color = {{r, g, b, 1.0f}};

    VkRenderPassBeginInfo rpBI{};
    rpBI.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBI.renderPass        = m_renderPass->handle();
    rpBI.framebuffer       = m_framebuffers[imageIndex];
    rpBI.renderArea.offset = {0, 0};
    rpBI.renderArea.extent = m_swapchain->extent();
    rpBI.clearValueCount   = 1;
    rpBI.pClearValues      = &clearColor;

    vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
    // (O Letterbox com VkViewport e VkScissor sera ativado aqui na Fase 5 
    // antes de começarmos a desenhar os sprites do jogo).
    vkCmdEndRenderPass(cmd);

    return vkEndCommandBuffer(cmd) == VK_SUCCESS;
}

} // namespace gfx