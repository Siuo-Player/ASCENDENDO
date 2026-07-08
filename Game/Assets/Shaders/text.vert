#version 450
// =============================================================================
//  Game/Assets/Shaders/text.vert
//
//  @version 7.6
//  Pipeline DEDICADA para texto (glyphs TTF). Separada da pipeline solida
//  (base.vert/base.frag) propositadamente: zero risco para a pipeline
//  existente/testada. UV mapeado para o atlas gerado por stb_truetype.
// =============================================================================
layout(push_constant) uniform PC {
    vec4 color;        // cor do texto (RGB usado, A multiplica o alpha do glyph)
    vec2 objPos;        // canto inferior-esquerdo do quad, espaco logico Y-cima
    vec2 objSize;
    vec2 uv0;           // s0,t0 — canto sup-esq do glyph no atlas (V para baixo)
    vec2 uv1;           // s1,t1 — canto inf-dir do glyph no atlas (V para baixo)
    vec2 logicalRes;
} pc;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;

void main() {
    const vec2 corners[6] = vec2[](
        vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
        vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
    );
    vec2 c = corners[gl_VertexIndex % 6];
    vec2 worldPos = pc.objPos + c * pc.objSize;

    gl_Position = vec4(
        (worldPos.x / pc.logicalRes.x) *  2.0 - 1.0,
        (worldPos.y / pc.logicalRes.y) * -2.0 + 1.0,   // Y invertido p/ Vulkan
        0.0, 1.0
    );

    fragColor = pc.color;
    // c.y=0 (base do quad) -> t1 (fundo do glyph no atlas, V-baixo)
    // c.y=1 (topo do quad) -> t0 (topo do glyph no atlas)
    fragUV = vec2(
        mix(pc.uv0.x, pc.uv1.x, c.x),
        mix(pc.uv1.y, pc.uv0.y, c.y)
    );
}
