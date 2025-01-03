add_library(pbr_engine STATIC)
target_compile_features(pbr_engine PUBLIC cxx_std_26)

target_link_libraries(pbr_engine PUBLIC
  Vulkan::Vulkan

  GPUOpen::VulkanMemoryAllocator
  glm::glm

  pbr_engine_vulkan_include
  pbr_engine_core
)

target_include_directories(pbr_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(pbr_engine PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/SwapchainImageView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/Surface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/AsyncSubmitter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/TransferStager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/PbrPipeline.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/MeshBuilder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/Material.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/Scene.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/memory/Allocation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pbr/memory/MemoryAllocator.cpp
)
