// shader_utils.h
#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>

namespace ShaderUtils {
    unsigned int make_shader(const std::string& vertex_filepath,
                            const std::string& fragment_filepath);
    unsigned int make_module(const std::string& filepath,
                            unsigned int module_type);
}