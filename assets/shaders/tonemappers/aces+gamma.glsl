#version 460

#include "../Gamma.lib.glsl"

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba16f, set = 0, binding = 0) readonly uniform image2D hdrImage;
layout(rgba8, set = 0, binding = 1) writeonly uniform image2D outImage;

const mat3x3 ACES_INPUT = mat3x3(
        0.59719, 0.35458, 0.04823,
        0.07600, 0.90834, 0.01566,
        0.02840, 0.13383, 0.83777
    );
const mat3x3 ACES_OUTPUT = mat3x3(
        1.60475, -0.53108, -0.07367,
        -0.10208, 1.10813, -0.00605,
        -0.00327, -0.07276, 1.07602
    );
vec3 RRTAndODTFit(vec3 color) {
    vec3 numerator = color * (color + 0.0245786) - 0.000090537;
    vec3 denominator = color * (0.983729 * color + 0.4329510) + 0.238081;
    return numerator / denominator;
}

vec3 acesTonemap(vec3 hdrColor) {
    return ACES_OUTPUT * RRTAndODTFit(ACES_INPUT * hdrColor);
}

void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(hdrImage);

    if (coords.x < size.x && coords.y < size.y) {
        vec4 hdrColor = imageLoad(hdrImage, coords);
        vec3 ldrColor = acesTonemap(hdrColor.rgb);
        vec3 gammaCorrected = gammaCorrect(ldrColor, 2.2);
        vec4 finalColor = clamp(vec4(gammaCorrected, hdrColor.a), vec4(0.0), vec4(1.0));
        imageStore(outImage, coords, finalColor);
    }
}
