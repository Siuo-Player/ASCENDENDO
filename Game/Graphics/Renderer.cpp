// =============================================================================
//  @version 7.5
//  @history
//    v5.2  — drawFrame(Player, Camera)
//    v5.3  — versao do .h atualizada
//    v6.2b — recordCommandBuffer desenha plataformas (nivel) antes do jogador
//    v7.5  — GameState: PLAYING (+ FLAG visual), CREDITS, MENU
// =============================================================================
#include "Graphics/Renderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Pipeline.h"
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Graphics/Camera.h"
#include "Core/Config.h"

namespace gfx {

bool Renderer::init(VulkanContext* ctx, Swapchain* swapchain,
                    RenderPass* renderPass, Pipeline* pipeline) {
    if (m_initialized) return true;
    if (!ctx || !swapchain || !renderPass || !pipeline) return false;
    if (!ctx->isInitialized() || !swapchain->isInitialized() ||
        !renderPass->isInitialized() || !pipeline->isInitialized()) return false;

    m_ctx = ctx; m_swapchain = swapchain;
    m_renderPass = renderPass; m_pipeline = pipeline;

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

    for (auto fb : m_framebuffers) vkDestroyFramebuffer(device, fb, nullptr);
    m_framebuffers.clear();

    if (m_inFlightFence)           vkDestroyFence(device, m_inFlightFence, nullptr);
    if (m_renderFinishedSemaphore) vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
    if (m_imageAvailableSemaphore) vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
    if (m_commandPool)             vkDestroyCommandPool(device, m_commandPool, nullptr);

    m_commandBuffers.clear();
    m_inFlightFence = VK_NULL_HANDLE;
    m_commandPool   = VK_NULL_HANDLE;
    m_ctx = nullptr; m_swapchain = nullptr;
    m_renderPass = nullptr; m_pipeline = nullptr;
    m_initialized = false;
}

bool Renderer::drawFrame(const logic::Player& player, const gfx::Camera& camera,
                         const logic::Level* level,
                         GameState state, int menuSel) {
    VkDevice device = m_ctx->device();
    vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device, m_swapchain->handle(), UINT64_MAX,
                                            m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return false;

    vkResetFences(device, 1, &m_inFlightFence);
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);

    if (!recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex,
                              player, camera, level, state, menuSel))
        return false;

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

    if (vkQueueSubmit(m_ctx->graphicsQueue(), 1, &submitInfo, m_inFlightFence) != VK_SUCCESS)
        return false;

    VkSwapchainKHR sc = m_swapchain->handle();
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &sc;
    presentInfo.pImageIndices      = &imageIndex;

    result = vkQueuePresentKHR(m_ctx->graphicsQueue(), &presentInfo);
    return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
}

bool Renderer::recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex,
                                   const logic::Player& player,
                                   const gfx::Camera& camera,
                                   const logic::Level* level,
                                   GameState state, int menuSel) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) return false;

    // ── Cor de fundo baseada no estado ────────────────────────────────────────
    VkClearValue clearColor{};
    if (state == GameState::CREDITS) {
        clearColor.color = {{config::CLEAR_CREDITS_R, config::CLEAR_CREDITS_G,
                              config::CLEAR_CREDITS_B, 1.0f}};
    } else if (state == GameState::MENU) {
        clearColor.color = {{config::CLEAR_MENU_R, config::CLEAR_MENU_G,
                              config::CLEAR_MENU_B, 1.0f}};
    } else {
        clearColor.color = {{0.05f, 0.05f, 0.15f, 1.0f}};
    }

    VkRenderPassBeginInfo rpBI{};
    rpBI.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBI.renderPass        = m_renderPass->handle();
    rpBI.framebuffer       = m_framebuffers[imageIndex];
    rpBI.renderArea.offset = {0, 0};
    rpBI.renderArea.extent = m_swapchain->extent();
    rpBI.clearValueCount   = 1;
    rpBI.pClearValues      = &clearColor;

    vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());

    // ── Letterbox viewport ────────────────────────────────────────────────────
    int32_t winW = m_swapchain->extent().width;
    int32_t winH = m_swapchain->extent().height;
    float   winAR = (float)winW / (float)winH;

    int32_t viewW = winW, viewH = winH;
    if (winAR > config::TARGET_ASPECT) viewW = (int32_t)(winH * config::TARGET_ASPECT);
    else                               viewH = (int32_t)(winW / config::TARGET_ASPECT);

    int32_t offsetX = (winW - viewW) / 2;
    int32_t offsetY = (winH - viewH) / 2;

    VkViewport viewport{};
    viewport.x = (float)offsetX; viewport.y = (float)offsetY;
    viewport.width = (float)viewW; viewport.height = (float)viewH;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {offsetX, offsetY};
    scissor.extent = {(uint32_t)viewW, (uint32_t)viewH};
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    const VkShaderStageFlags stages =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // ── Helper: desenhar retangulo no espaco logico ───────────────────────────
    // 'cam' = nullptr → camara neutra (UI em coordenadas logicas fixas)
    auto drawRect = [&](float x, float y, float w, float h,
                        float r, float g, float b, float a = 1.0f,
                        const gfx::Camera* cam = nullptr) {
        PushConstants p{};
        p.color[0] = r; p.color[1] = g; p.color[2] = b; p.color[3] = a;
        p.camPos[0]     = cam ? cam->position.x : 0.0f;
        p.camPos[1]     = cam ? cam->position.y : 0.0f;
        p.objPos[0]     = x; p.objPos[1] = y;
        p.objSize[0]    = w; p.objSize[1] = h;
        p.logicalRes[0] = config::LOGICAL_WIDTH;
        p.logicalRes[1] = config::LOGICAL_HEIGHT;
        vkCmdPushConstants(cmd, m_pipeline->layout(), stages, 0, sizeof(PushConstants), &p);
        vkCmdDraw(cmd, 6, 1, 0, 0);
    };

    // ═════════════════════════════════════════════════════════════════════════
    if (state == GameState::PLAYING) {
    // ═════════════════════════════════════════════════════════════════════════

        // ── 1. PLATAFORMAS ────────────────────────────────────────────────────
        if (level && !level->platforms().empty()) {
            for (const auto& platform : level->platforms()) {
                drawRect(platform.bounds.min.x, platform.bounds.min.y,
                         platform.bounds.width(), platform.bounds.height(),
                         config::COLOR_PLATFORM_R, config::COLOR_PLATFORM_G,
                         config::COLOR_PLATFORM_B, 1.0f, &camera);
            }
        }

        // ── 2. FLAG (mastro + pano dourado) ───────────────────────────────────
        if (level && level->hasFlag) {
            const auto& fb = level->flagBounds;
            float cx   = fb.min.x + (fb.width() - 4.0f) / 2.0f; // centro horizontal

            // Mastro (dourado escuro, 4px, altura total da AABB)
            drawRect(cx, fb.min.y, 4.0f, fb.height(),
                     config::COLOR_FLAG_POLE_R, config::COLOR_FLAG_POLE_G,
                     config::COLOR_FLAG_POLE_B, 1.0f, &camera);

            // Pano (dourado brilhante, 60% da largura, 35% superior)
            float panoH = fb.height() * 0.35f;
            drawRect(cx + 4.0f, fb.max.y - panoH,
                     fb.width() * 0.60f, panoH,
                     config::COLOR_FLAG_R, config::COLOR_FLAG_G,
                     config::COLOR_FLAG_B, 1.0f, &camera);
        }

        // ── 3. JOGADOR ────────────────────────────────────────────────────────
        drawRect(player.position().x, player.position().y,
                 player.body.width, player.body.height,
                 config::COLOR_PLAYER_R, config::COLOR_PLAYER_G,
                 config::COLOR_PLAYER_B, 1.0f, &camera);

    // ═════════════════════════════════════════════════════════════════════════
    } else if (state == GameState::CREDITS) {
    // ═════════════════════════════════════════════════════════════════════════
        // Coordenadas em espaco logico (640×360). Y=0=baixo, Y=360=cima.
        // Sem texto real: barras coloridas sugerem linhas de texto.

        // Titulo (dourado largo)
        drawRect(100, 295, 440, 26, 1.0f, 0.85f, 0.10f);
        // Subtitulo (branco dimmer)
        drawRect(100, 265, 440, 14, 0.75f, 0.75f, 0.80f);
        // Separador
        drawRect( 90, 250, 460,  2, 0.30f, 0.30f, 0.40f);

        // Autor
        drawRect(100, 220, 240, 14, 0.95f, 0.95f, 0.95f);
        // "Auxiliado por: Claude (Anthropic)"
        drawRect(100, 198, 310, 12, 0.70f, 0.80f, 1.00f);
        // "              Gemini (Google)"
        drawRect(100, 180, 270, 12, 0.60f, 0.90f, 0.78f);

        // Separador inferior
        drawRect( 90,  90, 460,  2, 0.25f, 0.25f, 0.35f);
        // "Pressiona ESPACO" (barra discreta a piscar — sempre visivel por agora)
        drawRect(180,  62, 280, 10, 0.35f, 0.35f, 0.50f);

    // ═════════════════════════════════════════════════════════════════════════
    } else if (state == GameState::MENU) {
    // ═════════════════════════════════════════════════════════════════════════
        // Titulo do menu
        drawRect(180, 280, 280, 22, 0.90f, 0.80f, 0.20f);
        // Separador
        drawRect( 80, 262, 480,  2, 0.25f, 0.25f, 0.35f);

        // Opcao 0: "Comecar" (esquerda)
        float sel0r = (menuSel == 0) ? 1.0f : 0.20f;
        float sel0g = (menuSel == 0) ? 0.85f : 0.20f;
        float sel0b = (menuSel == 0) ? 0.10f : 0.25f;
        drawRect( 80, 150, 200, 55, sel0r, sel0g, sel0b);
        // Icone interno (retangulo menor centrado, ≈ triangulo play)
        drawRect(140, 163, 80, 30, sel0r * 0.7f, sel0g * 0.7f, sel0b * 0.6f);

        // Opcao 1: "Creditos" (direita)
        float sel1r = (menuSel == 1) ? 1.0f : 0.20f;
        float sel1g = (menuSel == 1) ? 0.85f : 0.20f;
        float sel1b = (menuSel == 1) ? 0.10f : 0.25f;
        drawRect(360, 150, 200, 55, sel1r, sel1g, sel1b);
        drawRect(400, 163, 120, 10, sel1r * 0.7f, sel1g * 0.7f, sel1b * 0.6f);
        drawRect(400, 178, 100, 10, sel1r * 0.7f, sel1g * 0.7f, sel1b * 0.6f);
        drawRect(400, 193, 80,  10, sel1r * 0.7f, sel1g * 0.7f, sel1b * 0.6f);

        // Dica de navegacao (barra discreta)
        drawRect(160,  50, 320,  8, 0.25f, 0.25f, 0.35f);
    }

    vkCmdEndRenderPass(cmd);
    return vkEndCommandBuffer(cmd) == VK_SUCCESS;
}

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
        if (vkCreateFramebuffer(m_ctx->device(), &fbCI, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            return false;
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
    VkSemaphoreCreateInfo semCI{};  semCI.sType  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo     fenceCI{}; fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkDevice dev = m_ctx->device();
    return vkCreateSemaphore(dev, &semCI,  nullptr, &m_imageAvailableSemaphore) == VK_SUCCESS
        && vkCreateSemaphore(dev, &semCI,  nullptr, &m_renderFinishedSemaphore) == VK_SUCCESS
        && vkCreateFence    (dev, &fenceCI,nullptr, &m_inFlightFence)           == VK_SUCCESS;
}

} // namespace gfx
