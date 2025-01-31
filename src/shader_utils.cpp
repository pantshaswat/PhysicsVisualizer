#include "shader_utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>



namespace ShaderUtils {

unsigned int make_shader(const std::string& vertex_filepath,
                        const std::string& fragment_filepath) 
{
    std::vector<unsigned int> modules;
    modules.reserve(2);  // Pre-allocate for 2 shaders

    try {
        modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
        modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));
    } catch (const std::exception& e) {
        // Cleanup if shader creation failed
        for (auto module : modules) {
            glDeleteShader(module);
        }
        throw;
    }

    unsigned int shader = glCreateProgram();
    for (unsigned int shaderModule : modules) {
        glAttachShader(shader, shaderModule);
    }
    glLinkProgram(shader);

    // Always check linking status
    int success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetProgramInfoLog(shader, 1024, nullptr, errorLog);
        std::cerr << "Shader linking failed:\n" << errorLog << '\n';
        glDeleteProgram(shader);
        return 0;  // Return 0 to indicate failure
    }

    // Cleanup shaders after linking
    for (unsigned int shaderModule : modules) {
        glDetachShader(shader, shaderModule);
        glDeleteShader(shaderModule);
    }

    return shader;
}

unsigned int make_module(const std::string& filepath,
                        unsigned int module_type) 
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filepath);
    }

    std::stringstream bufferedLines;
    std::string line;
    while (std::getline(file, line)) {
        bufferedLines << line << '\n';
    }

    const std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str();

    unsigned int shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &shaderSrc, nullptr);
    glCompileShader(shaderModule);

    // Check compilation status
    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, nullptr, errorLog);
        glDeleteShader(shaderModule);
        throw std::runtime_error("Shader compilation failed: " + 
                                std::string(errorLog));
    }

    return shaderModule;
}

} // namespace ShaderUtils