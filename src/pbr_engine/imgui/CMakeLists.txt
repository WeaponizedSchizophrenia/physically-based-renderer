add_library(pbr_engine_imgui STATIC)
target_compile_features(pbr_engine_imgui PUBLIC cxx_std_26)

target_link_libraries(pbr_engine_imgui PUBLIC
    Vulkan::Vulkan

    imgui
    glm::glm

    pbr_engine_vulkan_include
    pbr_engine_core
    pbr_engine
)

target_include_directories(pbr_engine_imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(pbr_engine_imgui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/pbr/imgui/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pbr/imgui/Pipeline.cpp
)
