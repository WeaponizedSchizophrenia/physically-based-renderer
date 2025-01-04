#version 460

#include "../Camera.lib.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoords;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec3 outBitangent;
layout(location = 4) out vec2 outTexCoords;

layout(push_constant) uniform PC {
    mat4x4 model;
    mat3x3 normalModel;
} pc;

layout(set = 0, binding = 0) uniform CameraUBO {
    Camera cam;
};

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    gl_Position = cam.proj * cam.view * worldPos;
    outPosition = worldPos.xyz;
    outNormal = normalize(pc.normalModel * inNormal);
    outTangent = normalize(pc.normalModel * inTangent.xyz);
    outBitangent = normalize(pc.normalModel * (cross(inNormal, inTangent.xyz) * inTangent.w));
    outTexCoords = inTexCoords;
}
