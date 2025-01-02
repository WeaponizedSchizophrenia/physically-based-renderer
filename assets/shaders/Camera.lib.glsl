#ifndef CAMERA_LIB
#define CAMERA_LIB

struct Camera {
  mat4x4 view;
  mat4x4 proj;
  vec3 position;
};

#endif // CAMERA_LIB
