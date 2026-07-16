// =============================================================================
//  Game/Graphics/Renderer.cpp
//
//  @version 9.3
//  @history
//    v5.2  — drawFrame(Player, Camera)
//    v6.2b — plataformas desenhadas antes do jogador
//    v7.5  — GameState, FLAG visual melhorado, BitmapFont (drawText real)
//    v7.6  — attachText(): TTF real (FontRenderer+TextPipeline) em
//             CREDITS/MENU, com fallback para BitmapFont se nao anexado.
//    v8.1  — GameState::PAUSED (mundo congelado + overlay + menu 3 opcoes).
//             drawWorld() extraido como helper partilhado PLAYING/PAUSED.
//             Timer HUD em PLAYING. MENU (fim-de-run) agora tem 3 opcoes.
//    v8.2  — FIX: zona de touchdown da FLAG removida (misturava-se com o
//             fundo escuro, parecia bug de cor). FIX: timer HUD deslocado
//             para longe do topo do ecra (estava a ser cortado pelo
//             clipping de viewport — causa real do "texto garbled").
//             FIX: layout de CREDITOS recalculado (labels e nomes tinham
//             pouco espaco entre si). attachSprite(): jogador desenhado
//             como pixel-art (SpriteRenderer) com flip conforme
//             facingDirection; fallback para rectangulo solido.
//    v9.3  — GameState::EDITOR: grelha de fundo (Core/Viewport.h::
//             MenuBoxLayout tambem usado para calcular a grelha? nao --
//             so' para PAUSED/MENU; a grelha usa EDITOR_GRID_VISUAL_SPACING
//             directamente). PAUSED e MENU deixam de ter cada um a sua
//             copia inline da geometria das caixas -- passam a chamar
//             core::MenuBoxLayout::boxWidth()/boxX() (Core/Viewport.h),
//             resolvendo a duplicacao ja assinalada no dev_log da Fase
//             9.2. MENU ganha uma 4a caixa ("Editor"). Verificado por
//             calculo que count=3 (PAUSED) reproduz EXACTAMENTE os
//             valores anteriores (170px) -- zero alteracao visual ali.
// =============================================================================
#include "Graphics/Renderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Pipeline.h"
#include "Graphics/TextPipeline.h"
#include "Graphics/FontRenderer.h"
#include "Graphics/SpritePipeline.h"
#include "Graphics/SpriteRenderer.h"
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Graphics/Camera.h"
#include "Graphics/BitmapFont.h"   // fallback (usado se attachText() nao for chamado)
#include "Core/Config.h"
#include "Core/Viewport.h"        // MenuBoxLayout (Fase 9.3: fonte unica com o hit-test)
#include <string>
#include <cstdio>
#include <cmath>

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

void Renderer::attachText(TextPipeline* textPipeline, FontRenderer* font) {
    m_textPipeline = textPipeline;
    m_font         = font;
}

void Renderer::attachSprite(SpritePipeline* spritePipeline, SpriteRenderer* sprite) {
    m_spritePipeline = spritePipeline;
    m_sprite         = sprite;
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
                         GameState state, int menuSel, float elapsedSeconds) {
    VkDevice device = m_ctx->device();
    vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device, m_swapchain->handle(), UINT64_MAX,
                                            m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return false;

    vkResetFences(device, 1, &m_inFlightFence);
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);

    if (!recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex,
                              player, camera, level, state, menuSel, elapsedSeconds))
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
                                   GameState state, int menuSel, float elapsedSeconds) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) return false;

    // ── Cor de fundo por estado ───────────────────────────────────────────────
    // PAUSED usa a MESMA cor de fundo que PLAYING — o mundo continua visivel
    // por baixo do overlay escuro (ver mais abaixo).
    VkClearValue clearColor{};
    if      (state == GameState::CREDITS)
        clearColor.color = {{config::CLEAR_CREDITS_R, config::CLEAR_CREDITS_G, config::CLEAR_CREDITS_B, 1.0f}};
    else if (state == GameState::MENU)
        clearColor.color = {{config::CLEAR_MENU_R, config::CLEAR_MENU_G, config::CLEAR_MENU_B, 1.0f}};
    else if (state == GameState::EDITOR)
        clearColor.color = {{config::CLEAR_EDITOR_R, config::CLEAR_EDITOR_G, config::CLEAR_EDITOR_B, 1.0f}};
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

    // ── Helper: mundo do jogo (plataformas + FLAG + jogador) ──────────────────
    // Partilhado por PLAYING e PAUSED — em PAUSED o mundo fica visivel,
    // congelado, por baixo do overlay + menu (em vez de desaparecer).
    auto drawWorld = [&]() {
        if (level && !level->platforms().empty()) {
            for (const auto& plat : level->platforms())
                drawRect(plat.bounds.min.x, plat.bounds.min.y,
                         plat.bounds.width(), plat.bounds.height(),
                         config::COLOR_PLATFORM_R, config::COLOR_PLATFORM_G,
                         config::COLOR_PLATFORM_B, 1.0f, &camera);
        }
        if (level && level->hasFlag) {
            const auto& fb = level->flagBounds;
            float mid = fb.min.x + fb.width() * 0.5f;

            // v8.2: removida a zona de touchdown semi-transparente (alpha 0.18)
            // que aqui estava — sobre o fundo azul-escuro do jogo, um dourado
            // a 18% de alpha misturava-se para um castanho/oliva que parecia
            // um bug de cor errada. O mastro+pano ja comunicam bem onde fica
            // a FLAG; a plataforma solida por baixo (Level.cpp) já indica o
            // ponto de aterragem.
            drawRect(mid - 2.0f, fb.min.y, 4.0f, fb.height(),
                     config::COLOR_FLAG_POLE_R, config::COLOR_FLAG_POLE_G,
                     config::COLOR_FLAG_POLE_B, 1.0f, &camera);

            float panoW = fb.width() * 0.45f;
            float panoH = fb.height() * 0.42f;
            drawRect(mid + 2.0f, fb.max.y - panoH, panoW, panoH,
                     config::COLOR_FLAG_R, config::COLOR_FLAG_G,
                     config::COLOR_FLAG_B, 1.0f, &camera);
            drawRect(mid + 2.0f, fb.max.y - panoH + panoH*0.35f,
                     panoW, panoH*0.18f,
                     0.6f, 0.45f, 0.0f, 0.85f, &camera);
        }
        // JOGADOR: sprite pixel-art se disponivel, senao rectangulo solido.
        // O sprite precisa da SUA PROPRIA troca de pipeline (igual ao texto)
        // porque usa um VkPipelineLayout com descriptor set diferente do da
        // pipeline solida usada pelas plataformas/FLAG acima.
        if (m_sprite && m_spritePipeline) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_spritePipeline->handle());
            m_sprite->bind(cmd, m_spritePipeline->layout());
            bool flip = player.facingDirection < 0.0f;
            m_sprite->draw(cmd, m_spritePipeline->layout(),
                          player.position().x, player.position().y,
                          player.body.width, player.body.height, flip,
                          1.0f, 1.0f, 1.0f, 1.0f,
                          camera.position.x, camera.position.y);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());
        } else {
            drawRect(player.position().x, player.position().y,
                     player.body.width, player.body.height,
                     config::COLOR_PLAYER_R, config::COLOR_PLAYER_G,
                     config::COLOR_PLAYER_B, 1.0f, &camera);
        }
    };

    // Formata segundos como MM:SS para o timer HUD (sem depender de
    // RunHistory.h — Renderer nao deve conhecer detalhes de ficheiros).
    auto formatTimerMMSS = [](float seconds) -> std::string {
        if (seconds < 0.0f) seconds = 0.0f;
        int totalSec = (int)seconds;
        int sec = totalSec % 60;
        int min = totalSec / 60;
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d:%02d", min, sec);
        return std::string(buf);
    };

    // =========================================================================
    if (state == GameState::PLAYING) {
    // =========================================================================
        drawWorld();

        // Timer HUD — canto superior direito, espaco logico (sem camera).
        // v8.2 FIX: o baseline estava em LOGICAL_HEIGHT-12=348. Com scale=0.6
        // a altura do glyph (~28*0.6=16.8un) fazia o TOPO do texto chegar a
        // ~364.8, ULTRAPASSANDO o limite superior do ecra logico (360) — o
        // topo de cada digito era cortado pelo clipping de viewport/NDC,
        // dando o aspecto de caracteres garbled/errados. Corrigido: baseline
        // bem mais abaixo do topo (332), com margem folgada.
        if (m_font && m_textPipeline) {
            std::string t = formatTimerMMSS(elapsedSeconds);
            const float TIMER_SCALE = 0.7f;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textPipeline->handle());
            m_font->bind(cmd, m_textPipeline->layout());
            m_font->drawText(cmd, m_textPipeline->layout(), t.c_str(),
                             config::LOGICAL_WIDTH - m_font->textWidth(t.c_str(), TIMER_SCALE) - 16.0f,
                             config::LOGICAL_HEIGHT - 28.0f, TIMER_SCALE,
                             0.90f, 0.90f, 0.90f, 0.85f);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());
        }

    // =========================================================================
    } else if (state == GameState::PAUSED) {
    // =========================================================================
        drawWorld();   // mundo congelado, continua visivel

        // Overlay escuro semi-transparente sobre TODO o ecra logico.
        drawRect(0.0f, 0.0f, config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT,
                 0.0f, 0.0f, 0.0f, 0.55f);

        // Geometria via core::MenuBoxLayout (Fase 9.3) -- fonte unica com o
        // hit-test de main.cpp. Para count=3 reproduz EXACTAMENTE os
        // valores fixos anteriores (170px) -- confirmado por calculo antes
        // de aplicar esta mudanca (ver historico do ficheiro).
        const int   PAUSED_COUNT = 3;
        const float CX = config::LOGICAL_WIDTH / 2.0f;
        const float bW = core::MenuBoxLayout::boxWidth(PAUSED_COUNT, config::LOGICAL_WIDTH);
        const float bH = core::MenuBoxLayout::BOX_H;
        const float bY = core::MenuBoxLayout::BOX_Y;
        const char* labels[3] = { "CONTINUAR", "CREDITOS", "SAIR" };

        auto boxX = [&](int i) { return core::MenuBoxLayout::boxX(i, PAUSED_COUNT, config::LOGICAL_WIDTH); };

        if (m_font && m_textPipeline) {
            drawRect(60.0f, 245.0f, 520.0f, 2.0f, 0.35f, 0.35f, 0.45f);
            for (int i = 0; i < 3; ++i) {
                bool sel = (menuSel == i);
                float lr = sel ? 1.0f : 0.30f, lg = sel ? 0.85f : 0.30f, lb = sel ? 0.10f : 0.36f;
                drawRect(boxX(i), bY, bW, bH, lr*0.15f, lg*0.15f, lb*0.15f, 0.92f);
                drawRect(boxX(i),          bY,      bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY+bH-2, bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY,  2.0f, bH, lr, lg, lb);
                drawRect(boxX(i)+bW-2.0f,  bY,  2.0f, bH, lr, lg, lb);
            }

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textPipeline->handle());
            m_font->bind(cmd, m_textPipeline->layout());

            m_font->drawTextCentered(cmd, m_textPipeline->layout(), "PAUSA",
                                     CX, 275.0f, 1.3f, 1.0f, 0.85f, 0.10f, 1.0f);
            for (int i = 0; i < 3; ++i) {
                bool sel = (menuSel == i);
                float lr = sel ? 1.0f : 0.55f, lg = sel ? 0.85f : 0.55f, lb = sel ? 0.10f : 0.62f;
                m_font->drawTextCentered(cmd, m_textPipeline->layout(), labels[i],
                                         boxX(i) + bW/2.0f, bY + bH/2.0f - 6.5f, 0.5f, lr, lg, lb, 1.0f);
            }
            m_font->drawTextCentered(cmd, m_textPipeline->layout(),
                                     "A/D NAVEGAR   ESPACO CONFIRMAR   ESC CONTINUAR",
                                     CX, 100.0f, 0.42f, 0.60f, 0.60f, 0.70f, 1.0f);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());
        } else {
            // Fallback BitmapFont
            drawTextC("PAUSA", CX, 275.0f, 4.0f, 1.0f, 0.85f, 0.10f);
            drawRect(60.0f, 245.0f, 520.0f, 2.0f, 0.35f, 0.35f, 0.45f);
            for (int i = 0; i < 3; ++i) {
                bool sel = (menuSel == i);
                float lr = sel ? 1.0f : 0.30f, lg = sel ? 0.85f : 0.30f, lb = sel ? 0.10f : 0.36f;
                drawRect(boxX(i), bY, bW, bH, lr*0.15f, lg*0.15f, lb*0.15f, 0.92f);
                drawRect(boxX(i),          bY,      bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY+bH-2, bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY,  2.0f, bH, lr, lg, lb);
                drawRect(boxX(i)+bW-2.0f,  bY,  2.0f, bH, lr, lg, lb);
                drawTextC(labels[i], boxX(i)+bW/2.0f, bY+bH/2.0f-6.0f, 1.8f, lr, lg, lb);
            }
            drawTextC("A/D NAVEGAR  ESPACO CONFIRMAR  ESC CONTINUAR", CX, 100.0f, 1.4f, 0.5f,0.5f,0.6f);
        }

    // =========================================================================
    } else if (state == GameState::CREDITS) {
    // =========================================================================
        const float CX = config::LOGICAL_WIDTH / 2.0f;   // 320

        if (m_font && m_textPipeline) {
            // v8.2 FIX: layout recalculado por completo. O problema nao era
            // so o tamanho — era o ESPACAMENTO: um label pequeno (scale~0.55)
            // seguido de um nome grande (scale~0.85) com baselines a 22un de
            // distancia significa que o TOPO do nome (baseline+altura) fica
            // em cima do BASELINE do label acima (crowding/sobreposicao
            // visual). Recalculado com folga >=8un entre qualquer topo de
            // texto e o baseline do texto acima.

            // ── Passo 1: decoracoes (pipeline solida, ja vinculada) ────────────
            drawRect(60.0f, 250.0f, 520.0f, 2.0f, 0.30f, 0.30f, 0.40f);
            drawRect(60.0f,  82.0f, 520.0f, 2.0f, 0.25f, 0.25f, 0.35f);

            // ── Passo 2: texto TTF real (troca de pipeline UMA vez) ────────────
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textPipeline->handle());
            m_font->bind(cmd, m_textPipeline->layout());

            m_font->drawTextCentered(cmd, m_textPipeline->layout(), "ASCENDENDO",
                                     CX, 298.0f, 1.5f, 1.0f, 0.85f, 0.10f, 1.0f);
            m_font->drawTextCentered(cmd, m_textPipeline->layout(), "FIM DA CAMPANHA",
                                     CX, 266.0f, 0.75f, 0.80f, 0.80f, 0.80f, 1.0f);

            m_font->drawText(cmd, m_textPipeline->layout(), "AUTOR:",
                             80.0f, 225.0f, 0.45f, 0.50f, 0.50f, 0.60f, 1.0f);
            m_font->drawText(cmd, m_textPipeline->layout(), "RAFAEL GOMES BERNARDO",
                             80.0f, 193.0f, 0.70f, 0.95f, 0.95f, 0.95f, 1.0f);

            m_font->drawText(cmd, m_textPipeline->layout(), "AUXILIADO POR:",
                             80.0f, 160.0f, 0.45f, 0.50f, 0.50f, 0.60f, 1.0f);
            m_font->drawText(cmd, m_textPipeline->layout(), "CLAUDE  (ANTHROPIC)",
                             80.0f, 128.0f, 0.65f, 0.70f, 0.80f, 1.00f, 1.0f);
            m_font->drawText(cmd, m_textPipeline->layout(), "GEMINI  (GOOGLE)",
                             80.0f, 98.0f, 0.65f, 0.60f, 0.90f, 0.78f, 1.0f);

            m_font->drawTextCentered(cmd, m_textPipeline->layout(), "ESPACO PARA CONTINUAR",
                                     CX, 55.0f, 0.45f, 0.40f, 0.40f, 0.55f, 1.0f);

            // Volta a pipeline solida para o resto do frame (caso haja mais draws)
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());

        } else {
            // ── Fallback: fonte bitmap 5x5 (sem TextPipeline/FontRenderer) ────
            drawTextC("ASCENDENDO", CX, 300.0f, 5.0f, 1.0f, 0.85f, 0.10f);
            drawTextC("FIM DA CAMPANHA", CX, 268.0f, 3.0f, 0.80f, 0.80f, 0.80f);
            drawRect(60.0f, 255.0f, 520.0f, 2.0f, 0.30f, 0.30f, 0.40f);

            drawText("AUTOR:", 80.0f, 228.0f, 2.0f, 0.50f, 0.50f, 0.60f);
            drawText("RAFAEL GOMES BERNARDO", 80.0f, 206.0f, 3.0f, 0.95f, 0.95f, 0.95f);

            drawText("AUXILIADO POR:", 80.0f, 178.0f, 2.0f, 0.50f, 0.50f, 0.60f);
            drawText("CLAUDE  ANTHROPIC", 80.0f, 156.0f, 3.0f, 0.70f, 0.80f, 1.00f);
            drawText("GEMINI  GOOGLE",    80.0f, 128.0f, 3.0f, 0.60f, 0.90f, 0.78f);

            drawRect(60.0f, 90.0f, 520.0f, 2.0f, 0.25f, 0.25f, 0.35f);
            drawTextC("ESPACO PARA CONTINUAR", CX, 60.0f, 2.0f, 0.40f, 0.40f, 0.55f);
        }

    // =========================================================================
    } else if (state == GameState::MENU) {
    // =========================================================================
        const float CX = config::LOGICAL_WIDTH / 2.0f;

        // Fase 9.3: 4 caixas (era 3) -- ganhou "EDITOR". Geometria via
        // core::MenuBoxLayout (mesma fonte que PAUSED e o hit-test de
        // main.cpp) -- a largura encolhe automaticamente de 170 para
        // 146.5px para 4 caberem, PAUSED (count=3) fica inalterado.
        const int   MENU_COUNT = 4;
        const float bW = core::MenuBoxLayout::boxWidth(MENU_COUNT, config::LOGICAL_WIDTH);
        const float bH = core::MenuBoxLayout::BOX_H;
        const float bY = core::MenuBoxLayout::BOX_Y;
        const char* labels[4] = { "COMECAR", "EDITOR", "CREDITOS", "SAIR" };
        auto boxX = [&](int i) { return core::MenuBoxLayout::boxX(i, MENU_COUNT, config::LOGICAL_WIDTH); };

        if (m_font && m_textPipeline) {
            drawRect(60.0f, 287.0f, 520.0f, 2.0f, 0.28f, 0.28f, 0.38f);

            for (int i = 0; i < MENU_COUNT; ++i) {
                bool sel = (menuSel == i);
                float lr = sel ? 1.0f : 0.22f, lg = sel ? 0.85f : 0.22f, lb = sel ? 0.10f : 0.28f;
                drawRect(boxX(i), bY, bW, bH, lr*0.15f, lg*0.15f, lb*0.12f);
                drawRect(boxX(i),          bY,      bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY+bH-2, bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY,  2.0f, bH, lr, lg, lb);
                drawRect(boxX(i)+bW-2.0f,  bY,  2.0f, bH, lr, lg, lb);
            }

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textPipeline->handle());
            m_font->bind(cmd, m_textPipeline->layout());

            m_font->drawTextCentered(cmd, m_textPipeline->layout(), "ASCENDENDO",
                                     CX, 300.0f, 1.6f, 0.95f, 0.80f, 0.10f, 1.0f);

            // Escala de texto reduzida (0.5 -> 0.43, proporcional a 146.5/170)
            // porque as caixas encolheram para caberem as 4 -- "CREDITOS" (8
            // caracteres) e' o rotulo mais longo, o que mais aperta. Estimativa
            // proporcional, nao verificada visualmente (sem GPU na sandbox) --
            // ajustar aqui se overflow real no teu ecra.
            for (int i = 0; i < MENU_COUNT; ++i) {
                bool sel = (menuSel == i);
                float lr = sel ? 1.0f : 0.55f, lg = sel ? 0.85f : 0.55f, lb = sel ? 0.10f : 0.62f;
                m_font->drawTextCentered(cmd, m_textPipeline->layout(), labels[i],
                                         boxX(i) + bW/2.0f, bY + bH/2.0f - 6.5f, 0.43f, lr, lg, lb, 1.0f);
            }

            m_font->drawTextCentered(cmd, m_textPipeline->layout(),
                                     "A/D PARA NAVEGAR   ESPACO PARA CONFIRMAR",
                                     CX, 80.0f, 0.45f, 0.30f, 0.30f, 0.42f, 1.0f);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());

        } else {
            // ── Fallback: fonte bitmap 5x5 ──────────────────────────────────────
            drawTextC("ASCENDENDO", CX, 300.0f, 5.0f, 0.95f, 0.80f, 0.10f);
            drawRect(60.0f, 287.0f, 520.0f, 2.0f, 0.28f, 0.28f, 0.38f);

            for (int i = 0; i < MENU_COUNT; ++i) {
                bool sel = (menuSel == i);
                float lr = sel ? 1.0f : 0.22f, lg = sel ? 0.85f : 0.22f, lb = sel ? 0.10f : 0.28f;
                drawRect(boxX(i), bY, bW, bH, lr*0.15f, lg*0.15f, lb*0.12f);
                drawRect(boxX(i),          bY,      bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY+bH-2, bW, 2.0f, lr, lg, lb);
                drawRect(boxX(i),          bY,  2.0f, bH, lr, lg, lb);
                drawRect(boxX(i)+bW-2.0f,  bY,  2.0f, bH, lr, lg, lb);
                drawTextC(labels[i], boxX(i)+bW/2.0f, bY+bH/2.0f-6.0f, 1.55f, lr, lg, lb);
            }

            drawTextC("A/D PARA NAVEGAR   ESPACO PARA CONFIRMAR",
                      CX, 80.0f, 2.0f, 0.30f, 0.30f, 0.42f);
        }

    // =========================================================================
    } else if (state == GameState::EDITOR) {
    // =========================================================================
        // Grelha de fundo (referencia visual, Fase 9.3). Espacamento FIXO
        // (EDITOR_GRID_VISUAL_SPACING) -- deliberadamente independente de
        // EDITOR_GRID_SNAP (o snap de colocacao, Fase 9.4), que pode ser
        // muito mais fino (4px actualmente) do que faz sentido desenhar.
        const float spacing   = config::EDITOR_GRID_VISUAL_SPACING;
        const float thickness = 1.0f;
        const float gR = 0.24f, gG = 0.24f, gB = 0.30f;

        // Verticais: cobrem [camera.x, camera.x+LOGICAL_WIDTH], alinhadas
        // ao multiplo de `spacing` mais proximo por baixo da camara.
        float startX = std::floor(camera.position.x / spacing) * spacing;
        for (float x = startX; x <= camera.position.x + config::LOGICAL_WIDTH + spacing; x += spacing) {
            drawRect(x - thickness/2.0f, camera.position.y,
                     thickness, config::LOGICAL_HEIGHT, gR, gG, gB, 1.0f, &camera);
        }

        // Horizontais: idem em Y -- ambos os eixos culled a volta da
        // camara, por isso o custo por frame nao cresce com o tamanho da
        // campanha, so' com o que esta' visivel.
        float startY = std::floor(camera.position.y / spacing) * spacing;
        for (float y = startY; y <= camera.position.y + config::LOGICAL_HEIGHT + spacing; y += spacing) {
            drawRect(camera.position.x, y - thickness/2.0f,
                     config::LOGICAL_WIDTH, thickness, gR, gG, gB, 1.0f, &camera);
        }

        const float CX = config::LOGICAL_WIDTH / 2.0f;
        if (m_font && m_textPipeline) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textPipeline->handle());
            m_font->bind(cmd, m_textPipeline->layout());
            m_font->drawTextCentered(cmd, m_textPipeline->layout(), "EDITOR DE NIVEIS",
                                     CX, 330.0f, 0.55f, 0.75f, 0.80f, 0.85f, 1.0f);
            m_font->drawTextCentered(cmd, m_textPipeline->layout(),
                                     "A/D/W/S DESLOCAR CAMARA   ESC SAIR",
                                     CX, 30.0f, 0.40f, 0.45f, 0.48f, 0.52f, 1.0f);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->handle());
        } else {
            drawTextC("EDITOR DE NIVEIS", CX, 330.0f, 2.0f, 0.75f, 0.80f, 0.85f);
            drawTextC("A/D/W/S DESLOCAR CAMARA  ESC SAIR", CX, 30.0f, 1.2f, 0.45f, 0.48f, 0.52f);
        }
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
