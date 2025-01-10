# Compiles a shader with glslc
# Parameters:
#   shader_path - relative path (from assets/shaders) to the shader to compile
#   compiled_name - the name of the compiled shader
#   stage - the stage to pass into glslc
function(compileShader shader_path compiled_name stage)
    set(compiled_shader_path "compiled/${compiled_name}.spv")

    message("Compiling ${stage} shader ${shader_path} to ${compiled_shader_path}")

    execute_process(
      COMMAND glslc "-O" "-fshader-stage=${stage}" ${shader_path} "-o" ${compiled_shader_path}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/shaders
    )
endfunction()

function(compileAllShaders)
    message("Compiling shaders")
    # Fullscreen quad
    compileShader("fullscreen_vertex.glsl" "fullscreen_quad" "vertex")
    # Geometry pass
    compileShader("geometry_pass/vertex.glsl" "geometry_pass_vertex" "vertex")
    compileShader("geometry_pass/fragment.glsl" "geometry_pass_fragment" "fragment")
    # PBR
    compileShader("pbr/vertex.glsl" "pbr_vertex" "vertex")
    compileShader("pbr/fragment.glsl" "pbr_fragment" "fragment")
    compileShader("pbr/lighting.glsl" "pbr_lighting" "fragment")
    # Imgui
    compileShader("imgui/vertex.glsl" "imgui_vertex" "vertex")
    compileShader("imgui/fragment.glsl" "imgui_fragment" "fragment")
    # Tonemappers
    compileShader("tonemappers/aces+gamma.glsl" "tm_aces+gamma" "compute")
endfunction()
