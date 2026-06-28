// =============================================================================
//  Game/Graphics/Renderer.cpp
//
//  @version 7.5
//  @history
//    v5.2  — drawFrame(Player, Camera)
//    v6.2b — plataformas desenhadas antes do jogador
//    v7.5  — GameState, FLAG visual melhorado, BitmapFont (drawText real)
// =============================================================================
#include "Graphics/Renderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Pipeline.h"
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Graphics/Camera.h"
#include "Graphics/BitmapFont.h"   // fonte 5x5 bitmap
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

    // ── Cor de fundo por estado ───────────────────────────────────────────────
    VkClearValue clearColor{};
    if      (state == GameState::CREDITS)
        clearColor.color = {{config::CLEAR_CREDITS_R, config::CLEAR_CREDITS_G, config::CLEAR_CREDITS_B, 1.0f}};
    else if (state == GameState::MENU)
        clearColor.color = {{config::CLEAR_MENU_R, config::CLEAR_MENU_G, config::CLEAR_MENU_B, 1.0f}};
    else
        clearColor.color = {{0.05f, 0.05f, 0.15f, 1.0f}};

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
    int32_t winW = (int32_t)m_swapchain->extent().width;
    int32_t winH = (int32_t)m_swapchain->extent().height;
    float   winAR = (float)winW / (float)winH;
    int32_t viewW = winW, viewH = winH;
    if (winAR > config::TARGET_ASPECT) viewW = (int32_t)(winH * config::TARGET_ASPECT);
    else                               viewH = (int32_t)(winW / config::TARGET_ASPECT);
    int32_t offsetX = (winW - viewW) / 2;
    int32_t offsetY = (winH - viewH) / 2;

    VkViewport vp{}; vp.x=(float)offsetX; vp.y=(float)offsetY;
    vp.width=(float)viewW; vp.height=(float)viewH; vp.maxDepth=1.0f;
    vkCmdSetViewport(cmd, 0, 1, &vp);
    VkRect2D sc{}; sc.offset={offsetX,offsetY};
    sc.extent={(uint32_t)viewW,(uint32_t)viewH};
    vkCmdSetScissor(cmd, 0, 1, &sc);

    const VkShaderStageFlags stages =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // ── Helper: drawRect ──────────────────────────────────────────────────────
    // cam=nullptr → UI (coordenadas logicas sem camera)
    auto drawRect = [&](float x, float y, float w, float h,
                        float r, float g, float b, float a = 1.0f,
                        const gfx::Camera* cam = nullptr) {
        PushConstants p{};
        p.color[0]=r; p.color[1]=g; p.color[2]=b; p.color[3]=a;
        p.camPos[0]     = cam ? cam->position.x : 0.0f;
        p.camPos[1]     = cam ? cam->position.y : 0.0f;
        p.objPos[0]=x;  p.objPos[1]=y;
        p.objSize[0]=w; p.objSize[1]=h;
        p.logicalRes[0] = config::LOGICAL_WIDTH;
        p.logicalRes[1] = config::LOGICAL_HEIGHT;
        vkCmdPushConstants(cmd, m_pipeline->layout(), stages, 0, sizeof(PushConstants), &p);
        vkCmdDraw(cmd, 6, 1, 0, 0);
    };

    // ── Helper: drawGlyph / drawText (fonte bitmap 5x5) ──────────────────────
    // ps = tamanho de cada pixel da fonte em unidades logicas
    auto drawGlyph = [&](char c, float x, float y, float ps,
                          float r, float g, float b, float a = 1.0f,
                          const gfx::Camera* cam = nullptr) {
        auto uc = (unsigned char)(c>='a'&&c<='z' ? c-32 : c);
        const uint8_t* rows = gfx::FONT_5X5[uc & 0x7Fu];
        float gap = ps * 0.2f;   // pequena folga entre pixeis da fonte
        for (int row = 0; row < 5; ++row) {
            uint8_t bits = rows[row];
            if (!bits) continue;
            for (int col = 0; col < 5; ++col) {
                if (bits & (0x10u >> col)) {
                    drawRect(x + col*(ps+gap),
                             y + (4-row)*(ps+gap),
                             ps, ps, r, g, b, a, cam);
                }
            }
        }
    };

    // Largura de uma string com dado ps (para centrar)
    auto textWidth = [&](const char* text, float ps) -> float {
        float w = 0.0f; float stride = ps*5 + ps*1.2f;
        for (const char* p = text; *p; ++p)
            w += (*p==' ') ? ps*3.0f : stride;
        return w;
    };

    auto drawText = [&](const char* text, float x, float y, float ps,
                         float r, float g, float b, float a = 1.0f,
                         const gfx::Camera* cam = nullptr) {
        float cx = x; float stride = ps*5 + ps*1.2f;
        for (const char* p = text; *p; ++p) {
            if (*p==' ') { cx += ps*3.0f; continue; }
            drawGlyph(*p, cx, y, ps, r, g, b, a, cam);
            cx += stride;
        }
    };

    auto drawTextC = [&](const char* text, float cx, float y, float ps,
                          float r, float g, float b, float a = 1.0f,
                          const gfx::Camera* cam = nullptr) {
        drawText(text, cx - textWidth(text,ps)/2.0f, y, ps, r, g, b, a, cam);
    };

    // =========================================================================
    if (state == GameState::PLAYING) {
    // =========================================================================

        // 1. PLATAFORMAS (verdes)
        if (level && !level->platforms().empty()) {
            for (const auto& plat : level->platforms())
                drawRect(plat.bounds.min.x, plat.bounds.min.y,
                         plat.bounds.width(), plat.bounds.height(),
                         config::COLOR_PLATFORM_R, config::COLOR_PLATFORM_G,
                         config::COLOR_PLATFORM_B, 1.0f, &camera);
        }

        // 2. FLAG (mastro + pano) com zona de touchdown semi-transparente
        if (level && level->hasFlag) {
            const auto& fb = level->flagBounds;
            float mid = fb.min.x + fb.width() * 0.5f;  // centro horizontal

            // Zona de touchdown (dourado muito transparente — ajuda a ver a area)
            drawRect(fb.min.x, fb.min.y, fb.width(), fb.height(),
                     0.9f, 0.75f, 0.05f, 0.18f, &camera);

            // Mastro: 4px de largura, centrado horizontalmente, altura total
            drawRect(mid - 2.0f, fb.min.y, 4.0f, fb.height(),
                     config::COLOR_FLAG_POLE_R, config::COLOR_FLAG_POLE_G,
                     config::COLOR_FLAG_POLE_B, 1.0f, &camera);

            // Pano: 45% da largura total, 40% superior, a DIREITA do mastro
            float panoW = fb.width() * 0.45f;
            float panoH = fb.height() * 0.42f;
            drawRect(mid + 2.0f, fb.max.y - panoH, panoW, panoH,
                     config::COLOR_FLAG_R, config::COLOR_FLAG_G,
                     config::COLOR_FLAG_B, 1.0f, &camera);

            // Faixa escura no pano (detalhe visual)
            drawRect(mid + 2.0f, fb.max.y - panoH + panoH*0.35f,
                     panoW, panoH*0.18f,
                     0.6f, 0.45f, 0.0f, 0.85f, &camera);
        }

        // 3. JOGADOR
        drawRect(player.position().x, player.position().y,
                 player.body.width, player.body.height,
                 config::COLOR_PLAYER_R, config::COLOR_PLAYER_G,
                 config::COLOR_PLAYER_B, 1.0f, &camera);

    // =========================================================================
    } else if (state == GameState::CREDITS) {
    // =========================================================================
        // Todas as coordenadas em espaco logico 640x360 (camera=0,0)
        const float CX = config::LOGICAL_WIDTH / 2.0f;   // 320

        // Titulo principal
        drawTextC("ASCENDENDO", CX, 300.0f, 5.0f, 1.0f, 0.85f, 0.10f);
        // Subtitulo
        drawTextC("FIM DA CAMPANHA", CX, 268.0f, 3.0f, 0.80f, 0.80f, 0.80f);
        // Separador
        drawRect(60.0f, 255.0f, 520.0f, 2.0f, 0.30f, 0.30f, 0.40f);

        // Autor
        drawText("AUTOR:", 80.0f, 228.0f, 2.0f, 0.50f, 0.50f, 0.60f);
        drawText("RAFAEL GOMES BERNARDO", 80.0f, 206.0f, 3.0f, 0.95f, 0.95f, 0.95f);

        // Assistentes
        drawText("AUXILIADO POR:", 80.0f, 178.0f, 2.0f, 0.50f, 0.50f, 0.60f);
        drawText("CLAUDE  ANTHROPIC", 80.0f, 156.0f, 3.0f, 0.70f, 0.80f, 1.00f);
        drawText("GEMINI  GOOGLE",    80.0f, 128.0f, 3.0f, 0.60f, 0.90f, 0.78f);

        // Separador inferior
        drawRect(60.0f, 90.0f, 520.0f, 2.0f, 0.25f, 0.25f, 0.35f);
        // Dica
        drawTextC("ESPACO PARA CONTINUAR", CX, 60.0f, 2.0f, 0.40f, 0.40f, 0.55f);

    // =========================================================================
    } else if (state == GameState::MENU) {
    // =========================================================================
        const float CX = config::LOGICAL_WIDTH / 2.0f;

        // Titulo
        drawTextC("ASCENDENDO", CX, 300.0f, 5.0f, 0.95f, 0.80f, 0.10f);
        drawRect(60.0f, 287.0f, 520.0f, 2.0f, 0.28f, 0.28f, 0.38f);

        // Funcao utilitaria: desenhar caixa de opcao com borda e texto
        auto drawOption = [&](const char* label, float boxX, bool selected) {
            const float bW=220, bH=80, bY=160;
            float lr = selected ? 1.0f : 0.22f;
            float lg = selected ? 0.85f: 0.22f;
            float lb = selected ? 0.10f: 0.28f;
            // Fundo escuro
            drawRect(boxX, bY, bW, bH, lr*0.15f, lg*0.15f, lb*0.12f);
            // Bordas (4 rectangulos = frame)
            drawRect(boxX,          bY,      bW, 2.0f, lr, lg, lb); // baixo
            drawRect(boxX,          bY+bH-2, bW, 2.0f, lr, lg, lb); // cima
            drawRect(boxX,          bY,  2.0f, bH, lr, lg, lb);     // esq
            drawRect(boxX+bW-2.0f,  bY,  2.0f, bH, lr, lg, lb);     // dir
            // Label centrado horizontalmente na caixa
            drawTextC(label, boxX + bW/2.0f, bY + bH/2.0f - 7.5f,
                      3.0f, lr, lg, lb);
        };

        drawOption("COMECAR",  60.0f,  menuSel == 0);   // opcao esquerda
        drawOption("CREDITOS", 360.0f, menuSel == 1);   // opcao direita

        // Indicador de seleccao (seta/triangulo simulado por rectangulos)
        float arrowX = (menuSel == 0) ? 170.0f : 470.0f;
        drawRect(arrowX-4.0f, 148.0f, 8.0f, 8.0f, 1.0f, 0.85f, 0.10f); // ponta

        // Dica de navegacao
        drawTextC("A/D PARA NAVEGAR   ESPACO PARA CONFIRMAR",
                  CX, 80.0f, 2.0f, 0.30f, 0.30f, 0.42f);
        drawTextC("ESC PARA SAIR",
                  CX, 58.0f, 2.0f, 0.25f, 0.25f, 0.35f);
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
