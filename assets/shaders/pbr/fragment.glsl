#version 460

#include "../Camera.lib.glsl"
#include "../BlinnPhong.lib.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(set = 0, binding = 0) uniform CameraUBO {
    Camera cam;
};

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 color;
} mat;
layout(set = 1, binding = 1) uniform sampler2D colorSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;

void main() {
    mat3 tbn = mat3(inTangent, inBitangent, inNormal);
    vec3 normal = texture(normalSampler, inTexCoords).xyz * 2.0 - 1.0;
    normal = tbn * normal;
    vec4 color = texture(colorSampler, inTexCoords) * mat.color;

    vec3 V = normalize(cam.position - inPosition);
    vec3 L = V;

    outColor = blinnPhongLighting(color, normal, V, L);
}
