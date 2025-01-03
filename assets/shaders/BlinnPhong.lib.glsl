#ifndef BLINN_PHONG_LIB
#define BLINN_PHONG_LIB

vec4 blinnPhongLighting(vec4 fragColor, vec3 N, vec3 V, vec3 L) {
    float NdotV = max(dot(N, V), 0.0);
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 64);
    return fragColor * NdotV + vec4(spec);
}

#endif // BLINN_PHONG_LIB
