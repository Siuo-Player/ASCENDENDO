#version 450
// =============================================================================
//  Game/Assets/Shaders/sprite.vert
//
//  @version 8.2
//  Pipeline dedicada para sprites (pixel art). Mesma estrutura de
//  TextPipeline (descriptor set + push constants), mas o fragment shader
//  amostra RGBA directamente (sem recolorir) e o sampler usa NEAREST
//  (preserva as arestas nitidas dos pixeis, ao contrario do LINEAR usado
//  para o atlas de fontes, onde suavizar e desejavel).
// =============================================================================
layout(push_constant) uniform PC {
    vec4  tint;         // multiplicado pela cor da textura (1,1,1,1 = sem alteracao)
    vec2  objPos;        // canto inferior-esquerdo do sprite, espaco logico Y-cima
    vec2  objSize;
    vec2  camPos;
    vec2  logicalRes;
    float flipX;        // 1.0 = espelhar horizontalmente (virar para a esquerda)
    float _pad;
} pc;

layout(location = 0) out vec4 fragTint;
layout(location = 1) out vec2 fragUV;

void main() {
    const vec2 corners[6] = vec2[](
        vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
        vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
    );
    vec2 c = corners[gl_VertexIndex % 6];

    vec2 worldPos  = pc.objPos + c * pc.objSize;
    vec2 screenPos = worldPos - pc.camPos;
    gl_Position = vec4(
        (screenPos.x / pc.logicalRes.x) *  2.0 - 1.0,
        (screenPos.y / pc.logicalRes.y) * -2.0 + 1.0,
        0.0, 1.0
    );

    fragTint = pc.tint;
    // UV normal: c.x=0->u=0 (esquerda), c.x=1->u=1 (direita). Com flipX,
    // inverte-se para a sprite "olhar" na direccao oposta sem precisar de
    // uma segunda imagem espelhada.
    float u = (pc.flipX > 0.5) ? (1.0 - c.x) : c.x;
    // V: imagem tem origem no canto sup-esq (convencao stb_image); c.y=1
    // (topo do quad, Y-cima) deve mostrar o TOPO da imagem (v=0).
    float v = 1.0 - c.y;
    fragUV = vec2(u, v);
}
