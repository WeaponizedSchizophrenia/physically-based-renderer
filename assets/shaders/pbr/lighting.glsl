#version 460

#include "../Camera.lib.glsl"
#include "../BlinnPhong.lib.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 inTexCoords;

layout(set = 0, binding = 0) uniform SceneUBO {
    Camera cam;
};
layout(set = 1, binding = 0) uniform texture2D texPositions;
layout(set = 1, binding = 1) uniform texture2D texNormals;
layout(set = 1, binding = 2) uniform texture2D texAlbedo;
layout(set = 1, binding = 3) uniform sampler gBufferSampler;
layout(set = 1, binding = 4) uniform sampler2D depthSampler;

void main() {
    vec4 position = texture(sampler2D(texPositions, gBufferSampler), inTexCoords);
    vec4 normal = texture(sampler2D(texNormals, gBufferSampler), inTexCoords);
    vec4 albedo = texture(sampler2D(texAlbedo, gBufferSampler), inTexCoords);
    float depth = texture(depthSampler, inTexCoords).r;

    vec3 V = normalize(cam.position - position.xyz);
    vec3 L = V;

    outColor = blinnPhongLighting(albedo, normal.xyz, V, L);
}
