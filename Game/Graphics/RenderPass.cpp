#include "RenderPass.h"
#include "VulkanContext.h"
#include "Swapchain.h"

namespace gfx {

RenderPass::~RenderPass() {
    cleanup();
}

void RenderPass::cleanup() {
    if (m_renderPass != VK_NULL_HANDLE && m_ctx != nullptr) {
        vkDestroyRenderPass(m_ctx->device(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
}

bool RenderPass::init(VulkanContext* ctx, Swapchain* swapchain) {
    if (!ctx || !swapchain || !ctx->isInitialized() || !swapchain->isInitialized()) return false;
    m_ctx = ctx;

    // 1. Descrever a "tela" (Color Attachment)
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain->imageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;    // Limpar o ecrã no início do frame
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // Guardar para mostrar no ecrã
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Layout ideal para o monitor

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // 2. O Subpass (operações gráficas)
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // 3. Regras de sincronização
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // 4. Criar o Render Pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_ctx->device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        return false;
    }

    return true;
}

} // namespace gfx