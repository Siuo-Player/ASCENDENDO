#version 450

vec2 positions[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
    vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
);

layout(push_constant) uniform PushConstants {
    vec4  color;
    vec2  camPos;
    vec2  objPos;
    vec2  objSize;
    vec2  logicalRes;
} push;

void main() {
    vec2 localPos  = positions[gl_VertexIndex] * push.objSize;
    vec2 worldPos  = push.objPos + localPos;
    vec2 screenPos = worldPos - push.camPos;

    vec2 ndc;
    ndc.x =  (screenPos.x / push.logicalRes.x) * 2.0 - 1.0;
    ndc.y = -((screenPos.y / push.logicalRes.y) * 2.0 - 1.0);

    gl_Position = vec4(ndc, 0.0, 1.0);
}