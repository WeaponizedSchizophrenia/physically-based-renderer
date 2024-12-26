add_executable(gltf_viewer Main.cpp)
target_compile_features(gltf_viewer PRIVATE cxx_std_26)

target_link_libraries(gltf_viewer PRIVATE vkfw)
