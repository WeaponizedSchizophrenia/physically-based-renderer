#ifndef GAMMA_LIB
#define GAMMA_LIB

vec3 gammaCorrect(vec3 color, float gamma) {
  return pow(color, vec3(1.0 / gamma));
}

#endif // GAMMA_LIB
