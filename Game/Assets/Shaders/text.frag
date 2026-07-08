#version 450
// =============================================================================
//  Game/Assets/Shaders/text.frag
//  @version 7.6
//  Atlas em R8_UNORM: canal R = cobertura do glyph (0..1). RGB vem da cor
//  do push constant; A final = A do push constant * cobertura amostrada.
// =============================================================================
layout(set = 0, binding = 0) uniform sampler2D fontAtlas;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    float coverage = texture(fontAtlas, fragUV).r;
    outColor = vec4(fragColor.rgb, fragColor.a * coverage);
}
