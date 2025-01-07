#version 460

layout(location = 0) out vec4 outPositions;
layout(location = 1) out vec4 outNormals;
layout(location = 2) out vec4 outAlbedo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoords;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 color;
} mat;
layout(set = 1, binding = 1) uniform sampler2D colorSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;

void main() {
    // Positions
    outPositions = vec4(inPosition, 0.0);

    // Normals
    mat3 tbn = mat3(inTangent, inBitangent, inNormal);
    vec3 normal = texture(normalSampler, inTexCoords).xyz * 2.0 - 1.0;
    normal = normalize(tbn * normal);
    outNormals = vec4(normal, 0.0);

    // Albedo
    outAlbedo = mat.color * texture(colorSampler, inTexCoords);
}
