#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

layout(push_constant, std430) uniform PushConstant {
    vec2 scale;
    vec2 translation;
} pc;

void main() {
    outUV = inUV;
    outColor = inColor;
    gl_Position = vec4(inPosition * pc.scale + pc.translation, 0.0, 1.0);
}
