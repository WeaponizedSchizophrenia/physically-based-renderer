add_library(pbr_engine_core STATIC)
target_compile_features(pbr_engine_core PUBLIC cxx_std_26)

target_link_libraries(pbr_engine_core PUBLIC Vulkan::Vulkan pbr_engine_vulkan_include pbr_engine_utils)

target_include_directories(pbr_engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(pbr_engine_core PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/core/GpuHandle.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/core/Swapchain.cpp
)
