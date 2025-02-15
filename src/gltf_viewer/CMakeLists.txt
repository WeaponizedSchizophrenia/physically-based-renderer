add_executable(gltf_viewer Main.cpp App.cpp)
target_compile_features(gltf_viewer PRIVATE cxx_std_26)

target_include_directories(gltf_viewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gltf_viewer PRIVATE
  Vulkan::Vulkan

  vkfw
  spdlog::spdlog
  glm::glm
  imgui

  pbr_engine_vulkan_include
  pbr_engine_utils
  pbr_engine_core
  pbr_engine
  pbr_engine_gltf
  pbr_engine_imgui
)
target_sources(gltf_viewer PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/PerformanceOverlay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/SceneTree.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/AppUi.cpp
)
