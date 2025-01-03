add_library(pbr_engine_image STATIC)
target_compile_features(pbr_engine_image PUBLIC cxx_std_26)

target_link_libraries(pbr_engine_image PUBLIC
  Vulkan::Vulkan

  glm::glm

  pbr_engine_vulkan_include
  pbr_engine_core
  pbr_engine
)

target_include_directories(pbr_engine_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(pbr_engine_image PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/image/LoadImage.cpp
)
