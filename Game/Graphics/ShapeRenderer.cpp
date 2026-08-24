// =============================================================================
// Game/Graphics/ShapeRenderer.cpp
// =============================================================================
#include "Graphics/ShapeRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Camera.h"
#include "Core/Config.h"

namespace gfx {

bool ShapeRenderer::bind(VkCommandBuffer cmd, const Pipeline& pipeline) const {
    if (cmd == VK_NULL_HANDLE || !pipeline.isInitialized()) return false;
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
    return true;
}

void ShapeRenderer::drawRect(VkCommandBuffer cmd, const Pipeline& pipeline,
                             float x, float y, float w, float h,
                             float r, float g, float b, float a,
                             const Camera* camera) const {
    PushConstants constants{};
    constants.color[0] = r;
    constants.color[1] = g;
    constants.color[2] = b;
    constants.color[3] = a;
    constants.camPos[0] = camera ? camera->position.x : 0.0f;
    constants.camPos[1] = camera ? camera->position.y : 0.0f;
    constants.objPos[0] = x;
    constants.objPos[1] = y;
    constants.objSize[0] = w;
    constants.objSize[1] = h;
    constants.logicalRes[0] = config::LOGICAL_WIDTH;
    constants.logicalRes[1] = config::LOGICAL_HEIGHT;

    constexpr VkShaderStageFlags stages =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    vkCmdPushConstants(cmd, pipeline.layout(), stages, 0,
                       sizeof(PushConstants), &constants);
    vkCmdDraw(cmd, 6, 1, 0, 0);
}

} // namespace gfx
