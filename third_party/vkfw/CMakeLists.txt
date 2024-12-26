add_library(vkfw INTERFACE)
target_compile_features(vkfw INTERFACE cxx_std_20)
target_compile_definitions(vkfw INTERFACE
    VKFW_NO_STRUCT_CONSTRUCTORS
    # Disable smart handle because of a compilation error
    VKFW_NO_SMART_HANDLE
)

target_link_libraries(vkfw INTERFACE Vulkan::Headers glfw)

target_include_directories(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include)
target_precompile_headers(vkfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkfw/include/vkfw/vkfw.hpp)
