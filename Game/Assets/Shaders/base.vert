#version 450
// =============================================================================
//  Game/Assets/Shaders/base.vert
//
//  @version 5.1
// =============================================================================

// Vamos desenhar retângulos. Para evitar enviar vértices da CPU, 
// a GPU vai gerar os 6 cantos do retângulo (2 triângulos) dinamicamente.
// Posições base de um Quad normalizado [0.0 a 1.0]
vec2 positions[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
    vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
);

// Dados injetados diretamente da CPU via C++ a cada draw call
layout(push_constant) uniform PushConstants {
    vec2  camPos;        // Posição do canto inferior esquerdo da câmera
    vec2  objPos;        // Posição física do objeto no mundo
    vec2  objSize;       // Largura e Altura do objeto
    vec4  color;         // Cor do objeto
} push;

void main() {
    // 1. Pegar num vértice base [0,0 ou 1,1] e esticá-lo para o tamanho real
    vec2 localPos = positions[gl_VertexIndex] * push.objSize;
    
    // 2. Colocá-lo no lugar certo do mundo físico
    vec2 worldPos = push.objPos + localPos;
    
    // 3. Subtrair a Câmera (Matemática da nossa classe Camera)
    vec2 screenPos = worldPos - push.camPos;
    
    // 4. Converter para as NDC do Vulkan (-1.0 a 1.0, com o Y a crescer para baixo)
    vec2 ndc;
    ndc.x =  (screenPos.x / 360.0) * 2.0 - 1.0;
    ndc.y = -((screenPos.y / 640.0) * 2.0 - 1.0);
    
    gl_Position = vec4(ndc, 0.0, 1.0);
}