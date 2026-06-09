#version 450
// =============================================================================
//  Game/Assets/Shaders/base.frag
//
//  @version 5.1
// =============================================================================

layout(push_constant) uniform PushConstants {
    vec2  camPos;
    vec2  objPos;
    vec2  objSize;
    vec4  color; // RGBA
} push;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = push.color;
}