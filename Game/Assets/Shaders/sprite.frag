#version 450
// =============================================================================
//  Game/Assets/Shaders/sprite.frag
//  @version 8.2
//  Amostra RGBA diretamente do atlas de sprites — SEM recolorir (ao
//  contrario de text.frag, que usa o canal R como mascara de alpha e
//  aplica uma cor externa). tint permite efeitos futuros (flash de dano,
//  fade) sem precisar de nova pipeline.
// =============================================================================
layout(set = 0, binding = 0) uniform sampler2D spriteAtlas;

layout(location = 0) in vec4 fragTint;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 texel = texture(spriteAtlas, fragUV);
    outColor = texel * fragTint;
}
