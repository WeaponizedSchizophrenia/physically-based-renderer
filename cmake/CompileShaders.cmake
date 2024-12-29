# Compiles a shader with glslc
# Parameters:
#   shader_path - relative path (from assets/shaders) to the shader to compile
#   compiled_name - the name of the compiled shader
#   stage - the stage to pass into glslc
function(compileShader shader_path compiled_name stage)
    set(compiled_shader_path "compiled/${compiled_name}.spv")

    message("Compiling ${stage} shader ${shader_path} to ${compiled_shader_path}")

    execute_process(
        COMMAND glslc "-fshader-stage=${stage}" ${shader_path} "-o" ${compiled_shader_path}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/shaders
    )
endfunction()

function(compileAllShaders)
    message("Compiling shaders")
    compileShader("pbr/vertex.glsl" "pbr_vertex" "vertex")
    compileShader("pbr/fragment.glsl" "pbr_fragment" "fragment")
endfunction()
