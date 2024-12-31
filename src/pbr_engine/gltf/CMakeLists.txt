add_library(pbr_engine_gltf STATIC)
target_compile_features(pbr_engine_gltf PUBLIC cxx_std_26)

target_link_libraries(pbr_engine_gltf PUBLIC
  Vulkan::Vulkan

  GPUOpen::VulkanMemoryAllocator
  glm::glm
  fastgltf::fastgltf

  pbr_engine_vulkan_include
  pbr_engine_core
  pbr_engine
)

target_include_directories(pbr_engine_gltf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(pbr_engine_gltf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/gltf/Loader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/gltf/Asset.cpp
)
