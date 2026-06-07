// =============================================================================
//  Game/Graphics/Renderer.cpp
//
//  @version 2.6
//  @history
//    v2.6 — criado
// =============================================================================

#include "Graphics/Renderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"

namespace gfx {

// =============================================================================
//  Ciclo de Vida
// =============================================================================

bool Renderer::init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass) {
    if (m_initialized) return true;
    if (!ctx || !swapchain || !renderPass) return false;
    if (!ctx->isInitialized() || !swapchain->isInitialized() || !renderPass->isInitialized()) {
        return false;
    }

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
    vkDeviceWaitIdle(device); // Esperar que a GPU termine antes de destruir

    // ── Sincronizacao ─────────────────────────────────────────────────────────
    if (m_inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(device, m_inFlightFence, nullptr);
        m_inFlightFence = VK_NULL_HANDLE;
    }
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    // ── Command Pool (liberta os Command Buffers automaticamente) ─────────────
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
    m_commandBuffers.clear();

    // ── Framebuffers ──────────────────────────────────────────────────────────
    for (auto fb : m_framebuffers) {
        if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(device, fb, nullptr);
    }
    m_framebuffers.clear();

    m_ctx        = nullptr;
    m_swapchain  = nullptr;
    m_renderPass = nullptr;
    m_initialized = false;
}

// =============================================================================
//  Render
// =============================================================================

bool Renderer::drawFrame(float r, float g, float b) {
    VkDevice device = m_ctx->device();

    // 1. Esperar pelo frame anterior (evita sobrescrever recursos em uso)
    vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    // 2. Adquirir proxima imagem do swapchain
    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(
        device, m_swapchain->handle(), UINT64_MAX,
        m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return false;

    // 3. Resetar fence antes de submeter (so depois do acquire ter sucesso)
    vkResetFences(device, 1, &m_inFlightFence);

    // 4. Gravar commandos para este frame
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
    if (!recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex, r, g, b)) {
        return false;
    }

    // 5. Submeter para a fila grafica
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

    if (vkQueueSubmit(m_ctx->graphicsQueue(), 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        return false;
    }

    // 6. Apresentar no ecra
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

// =============================================================================
//  Inicializacao interna
// =============================================================================

bool Renderer::createFramebuffers() {
    const auto& views = m_swapchain->imageViews();
    m_framebuffers.resize(views.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < views.size(); ++i) {
        VkFramebufferCreateInfo fbCI{};
        fbCI.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCI.renderPass      = m_renderPass->handle();
        fbCI.attachmentCount = 1;
        fbCI.pAttachments    = &views[i];
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
    m_commandBuffers.resize(m_framebuffers.size());

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
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Comeca sinalizada — evita espera infinita no 1. frame

    VkDevice dev = m_ctx->device();
    return vkCreateSemaphore(dev, &semCI, nullptr, &m_imageAvailableSemaphore) == VK_SUCCESS
        && vkCreateSemaphore(dev, &semCI, nullptr, &m_renderFinishedSemaphore) == VK_SUCCESS
        && vkCreateFence(dev, &fenceCI, nullptr, &m_inFlightFence)             == VK_SUCCESS;
}

bool Renderer::recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex,
                                    float r, float g, float b) {
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
    // Fase 2.6: apenas clear. Fase 3+: adicionar draw calls aqui.
    vkCmdEndRenderPass(cmd);

    return vkEndCommandBuffer(cmd) == VK_SUCCESS;
}

} // namespace gfx
