#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<vector>
class SimulationBase {
public:
    virtual void init() = 0;
    virtual void render(float deltaTime) = 0;
    virtual void handleInput() = 0;
    virtual ~SimulationBase() = default;
    GLuint getShaderProgram() const { return shaderProgram; }

    
protected:
    virtual void setupBuffers();
    GLuint VAO, VBO, shaderProgram;
    
};