#include "projectile_simulation.h"
#include "shader_utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/include/imgui.h"
#include "imgui/include/imgui_impl_glfw.h"
#include "imgui/include/imgui_impl_opengl3.h"

// Shader paths
constexpr auto VERTEX_SHADER_PATH = "../shaders/projectile.vert";
constexpr auto FRAGMENT_SHADER_PATH = "../shaders/projectile.frag";

void ProjectileSimulation::init() {
    // 1. Set up shaders
    shaderProgram = ShaderUtils::make_shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

   
    
    // 2. Configure OpenGL buffers
    setupBuffers();
    setupCannonBuffers();    // Projectile-specific buffers
    setupTargetBuffers();
    
    // 3. Initialize projectile state
    resetSimulation();
}

void ProjectileSimulation::setupCannonBuffers(){
    //cannon vertices
    float scale = 2.0f;

   // Simple cannon vertices 
    float cannonVertices[] = {
        // Barrel (rectangle)
        -0.1f * scale,  0.1f * scale,    // Top left
         0.5f * scale,  0.1f * scale,    // Top right
         0.5f * scale, -0.1f * scale,    // Bottom right
        -0.1f * scale, -0.1f * scale,    // Bottom left
        
        // Base (wider rectangle)
        -0.3f * scale, -0.15f * scale,   // Bottom left
         0.0f * scale, -0.15f * scale,   // Bottom right
         0.0f * scale,  0.15f * scale,   // Top right
        -0.3f * scale,  0.15f * scale    // Top left
    };


    // cannon buffers
    glGenVertexArrays(1, &cannonVAO);
    glGenBuffers(1, &cannonVBO);

    glBindVertexArray(cannonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cannonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cannonVertices), cannonVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}
void ProjectileSimulation::setupTargetBuffers(){
    //target vertices
    std::vector<float> targetVertices;
    const int segments = 32;
    const float radius = 0.5f;

    for(int i = 0; i < segments; i++) {
        float angle = glm::radians(360.0f * i / segments);
        targetVertices.push_back(radius * cos(angle));
        targetVertices.push_back(radius * sin(angle));
    }
    // cannon buffers
    glGenVertexArrays(1, &targetVAO);
    glGenBuffers(1, &targetVBO);

    glBindVertexArray(targetVAO);
    glBindBuffer(GL_ARRAY_BUFFER, targetVBO);
    glBufferData(GL_ARRAY_BUFFER, targetVertices.size() * sizeof(float), targetVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void ProjectileSimulation::updatePhysics(float deltaTime) {
    if (!simulationRunning) return;

    constexpr float GRAVITY = 9.81f;
    
    projectile.time += deltaTime;
    projectile.position.x = projectile.startPosition.x + projectile.velocity.x * projectile.time;
    projectile.position.y = projectile.startPosition.y + 
                           (projectile.velocity.y * projectile.time) - 
                           (0.5f * GRAVITY * projectile.time * projectile.time);

    // Update statistics
    currentHeight = projectile.position.y;
    if (currentHeight > maxHeight) maxHeight = currentHeight;
    
    // Store path points (keep entire path)
    pathPoints.push_back(projectile.position);

    // Check for landing
    if (projectile.position.y <= 0.0f) {
        simulationRunning = false;
        simulationCompleted = true;  // Mark simulation as completed
        totalDistance = projectile.position.x - projectile.startPosition.x;
        distanceFromTarget = glm::abs(projectile.position.x - targetPosition.x);
    }
}

void ProjectileSimulation::render(float deltaTime) {
    // Update physics if simulation is running
    if (simulationRunning) {
        updatePhysics(deltaTime);
    }

    // Update vertex data with entire path
    std::vector<float> vertices;
    for (const auto& point : pathPoints) {
        vertices.push_back(point.x);
        vertices.push_back(point.y);
    }

    // Add ground path vertices
    
        vertices.push_back(pathPoints.front().x);
        vertices.push_back(projectile.startPosition.y); // Ground level
        vertices.push_back(pathPoints.back().x);
        vertices.push_back(projectile.startPosition.y); // Ground level
    

    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

     // Clear and set up rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);

        // Get uniform locations
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

        // Set projection matrix
    glm::mat4 projection = glm::ortho(-15.0f, 15.0f, -5.0f, 25.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Draw cannon
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(projectile.startPosition, 0.0f));
    model = glm::rotate(model, glm::radians(cannonAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    
    glBindVertexArray(cannonVAO);
    
    // Draw barrel
    glUniform3f(colorLoc, 0.4f, 0.4f, 0.4f); // Dark gray for barrel
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Draw base
    glUniform3f(colorLoc, 0.6f, 0.3f, 0.1f); // Brown for base
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    
    
    

        // Draw target
    model = glm::translate(glm::mat4(1.0f), glm::vec3(targetPosition, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // Red
    glBindVertexArray(targetVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 32);

        // Draw projectile path
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE,  &glm::mat4(1.0f)[0][0]);
    glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f); // Green for path
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, pathPoints.size());
    glDrawArrays(GL_POINTS, pathPoints.size() - 1, 1);



 

  

    // Draw ground path
     if (!pathPoints.empty()) {
        glUniform3f(colorLoc, 0.5f, 0.5f, 0.5f); // Gray for ground
        glDrawArrays(GL_LINES, pathPoints.size(), 2);
    }

    // Enhanced UI with initial conditions section
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation Controls");

    // Initial Conditions Section
    ImGui::TextColored(ImVec4(1,1,0,1), "INITIAL CONDITIONS");
    ImGui::Separator();
    ImGui::Spacing();

    // Only show controls when simulation is ready for new input
    if (!simulationRunning && !simulationCompleted) {
        ImGui::SliderFloat("Cannon Angle", &cannonAngle, 0.0f, 90.0f);
        ImGui::SliderFloat("Launch Speed", &launchSpeed, 0.0f, 50.0f);

            // Modified target distance input
    if (ImGui::SliderFloat("Target Distance", &targetDistance,0.0f,25.0f)) {
        targetPosition.x = projectile.startPosition.x + targetDistance;
    }
        
        if (ImGui::Button("Fire Cannon!", ImVec2(150, 30))) {
            float angleRad = glm::radians(cannonAngle);
            projectile.velocity.x = launchSpeed * cos(angleRad);
            projectile.velocity.y = launchSpeed * sin(angleRad);
            targetPosition.x = projectile.startPosition.x + targetDistance;
            simulationRunning = true;
        }
    }
    else if (simulationCompleted) {
        ImGui::Text("Simulation completed! Press Reset to start new simulation.");
        if (distanceFromTarget < 0.5f)
        {
            ImGui::TextColored(ImVec4(0,1,0,1), "Target Hit, Your point %.2f",std::max(0.0,10.0 - distanceFromTarget));
        }
        else
        {
            ImGui::TextColored(ImVec4(1,0,0,1), "Target Missed, Your point %.2f",std::max(0.0,10.0 - distanceFromTarget));
        }
        
        ImGui::Text("Distance from Target: %.2f m", distanceFromTarget);

    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Statistics Section
    ImGui::TextColored(ImVec4(1,1,0,1), "STATISTICS");
    ImGui::Text("Current Height: %.2f m", currentHeight);
    ImGui::Text("Max Height: %.2f m", maxHeight);
    ImGui::Text("Total Distance: %.2f m", totalDistance);
    ImGui::Text("Current Velocity: (%.2f, %.2f) m/s", projectile.velocity.x, projectile.velocity.y);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Controls Section
    ImGui::TextColored(ImVec4(1,1,0,1), "CONTROLS");
    if (ImGui::Button("Reset Simulation", ImVec2(150, 30))) {
        resetSimulation();
    }
        // Update target position when distance changes
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        targetPosition.x = projectile.startPosition.x + targetDistance;
    }
    
    ImGui::End();
}

void ProjectileSimulation::resetSimulation() {
    projectile = {
        .startPosition = glm::vec2(-10.0f, 0.0f),
        .position = glm::vec2(-10.0f, 0.0f),
        .velocity = glm::vec2(0.0f, 0.0f),
        .time = 0.0f
    };
    targetPosition = glm::vec2(projectile.startPosition.x + targetDistance, 0.0f);
    pathPoints.clear();
    pathPoints.push_back(projectile.position);
    simulationRunning = false;
    simulationCompleted = false;  // Reset completion flag
    maxHeight = 0.0f;
    totalDistance = 0.0f;
    currentHeight = 0.0f;
}

void ProjectileSimulation::handleInput() {
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_R) == GLFW_PRESS) {
        resetSimulation();
    }
}

ProjectileSimulation::~ProjectileSimulation() {
    glDeleteVertexArrays(1, &cannonVAO);
    glDeleteBuffers(1, &cannonVBO);
    glDeleteVertexArrays(1, &targetVAO);
    glDeleteBuffers(1, &targetVBO);

        // Clean up base class resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}