// =============================================================================
//  Game/Graphics/RenderPass.cpp
//
//  @version 4.2
// =============================================================================
#include "Graphics/RenderPass.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"

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

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = swapchain->imageFormat();
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    // Vai diretamente para o monitor (sem transferências intermediárias)
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    if (vkCreateRenderPass(m_ctx->device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        return false;
    }

    return true;
}

} // namespace gfx