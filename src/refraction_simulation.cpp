

#include "refraction_simulation.h"
#include "shader_utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/include/imgui.h"

constexpr auto VERTEX_SHADER_PATH = "../shaders/refraction.vert";
constexpr auto FRAGMENT_SHADER_PATH = "../shaders/refraction.frag";

void RefractionSimulation:: init(){
    // setup shaders
    shaderProgram = ShaderUtils::make_shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

    // config opengl buffers
    setupBuffers();  // for base class VAO, VBO
    setupInterfaceBuffers(); // for interface VAO,VBO

    // initialize
    resetSimulation();
}

void RefractionSimulation:: setupInterfaceBuffers(){
        interfacePoints = {
        glm::vec2(-10.0f, 0.0f),
        glm::vec2(10.0f, 0.0f)
    };

        glGenVertexArrays(1, &interfaceVAO);
    glGenBuffers(1, &interfaceVBO);
    
    glBindVertexArray(interfaceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, interfaceVBO);
    glBufferData(GL_ARRAY_BUFFER, interfacePoints.size() * sizeof(glm::vec2),
                interfacePoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void RefractionSimulation::calculateRefraction() {
            refractedRays.clear();
    refractionAngle = 0.0f;  
    reflectionAngle = 0.0f; 
    
    for (const auto& incident : incidentRays) {
 
    // Calculate where incident ray intersects with interface (y = 0)
        // Using parametric line equation: origin + t * direction = intersection
        // We know intersection.y = 0, so:
        // origin.y + t * direction.y = 0
        // t = -origin.y / direction.y
    
        float t = -incident.origin.y / incident.direction.y;
        glm::vec2 intersectionPoint = incident.origin + t * incident.direction;
        
        float theta1 = glm::radians(incidentAngle);
        float sinTheta2 = (n1 / n2) * sin(theta1);
        
        if (abs(sinTheta2) > 1.0f) {
            // Total internal reflection
            LightRay reflected;
            reflected.origin = intersectionPoint;
            reflected.direction = glm::reflect(incident.direction, glm::vec2(0.0f, 1.0f));
            reflected.intensity = incident.intensity;
            refractedRays.push_back(reflected);
            reflectionAngle = 180.0f - incidentAngle;  // Store reflection angle
            refractionAngle = -1.0f;  // Indicate total internal reflection
        } else {
            // Regular refraction
            float theta2 = asin(sinTheta2);
            LightRay refracted;
            refracted.origin = intersectionPoint;
            refracted.direction = glm::vec2(sin(theta2), -cos(theta2));
            refracted.intensity = incident.intensity * 0.8f;
            refractedRays.push_back(refracted);
            refractionAngle = glm::degrees(theta2);  // Store refraction angle
            reflectionAngle = -1.0f;  // Indicate no reflection
        }
    
    }
}
       
    



void RefractionSimulation::render(float deltaTime) {
    if (simulationRunning) {
        calculateRefraction();
    }
    
    // Update vertex data for all rays
    std::vector<float> vertices;
    
    // Add incident rays
    for (const auto& ray : incidentRays) {
        // Calculate intersection point with interface (y = 0)
        float t = -ray.origin.y / ray.direction.y;
        glm::vec2 intersectionPoint = ray.origin + t * ray.direction;
        
        // Add ray starting point
        vertices.push_back(ray.origin.x);
        vertices.push_back(ray.origin.y);
        // Add intersection point as endpoint
        vertices.push_back(intersectionPoint.x);
        vertices.push_back(intersectionPoint.y);
    }
    
    // Add refracted rays
    for (const auto& ray : refractedRays) {
        vertices.push_back(ray.origin.x);
        vertices.push_back(ray.origin.y);
        // Use longer length for refracted ray to ensure it's visible
        glm::vec2 endpoint = ray.origin + ray.direction * 10.0f;
        vertices.push_back(endpoint.x);
        vertices.push_back(endpoint.y);
    }
    
    // Rest of the render function remains the same...
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
    
    glm::mat4 projection = glm::ortho(-15.0f, 15.0f, -5.0f, 25.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
    
    // Render interface line
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(interfaceVAO);
    glDrawArrays(GL_LINES, 0, 2);
    
    // Update and render rays
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindVertexArray(VAO);
    
    // Draw incident rays (yellow)
    glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, incidentRays.size() * 2);
    
    // Draw refracted rays (cyan)
    glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, incidentRays.size() * 2, refractedRays.size() * 2);
    
    // ImGui controls
    ImGui::Begin("Refraction Controls");
    
    ImGui::TextColored(ImVec4(1,1,0,1), "SIMULATION PARAMETERS");
    ImGui::Separator();
    
    if (ImGui::SliderFloat("Incident Angle", &incidentAngle, 0.0f, 90.0f)) {
        updateRays();
    }
    
    ImGui::SliderFloat("n1 (Medium 1)", &n1, 1.0f, 2.0f);
    ImGui::SliderFloat("n2 (Medium 2)", &n2, 1.0f, 2.0f);

    ImGui::TextColored(ImVec4(1,1,0,1),"Angle of incidence: %1f", incidentAngle);
    
      if (refractionAngle >= 0) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Angle of refraction: %.1f°", refractionAngle);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Total Internal Reflection: %.1f°", incidentAngle);
    }
    
    if (ImGui::Button("Reset Simulation")) {
        resetSimulation();
    }
    
    ImGui::Checkbox("Show Critical Angle", &showCriticalAngle);
    if (showCriticalAngle) {
        criticalAngle = calculateCriticalAngle();
        ImGui::Text("Critical Angle: %.2f degrees", criticalAngle);
    }
    
    ImGui::End();
}

float RefractionSimulation::calculateCriticalAngle() {
    if (n1 <= n2) return 90.0f; // No critical angle exists
    return glm::degrees(asin(n2 / n1));
}

void RefractionSimulation::resetSimulation() {
    incidentRays.clear();
    refractedRays.clear();
    
    // Create initial incident ray
    LightRay initialRay;
    initialRay.origin = glm::vec2(0.0f, 5.0f);
    initialRay.direction = glm::vec2(sin(glm::radians(incidentAngle)), 
                                   -cos(glm::radians(incidentAngle)));
    initialRay.intensity = 1.0f;
    incidentRays.push_back(initialRay);
    
    simulationRunning = true;
}

void RefractionSimulation::handleInput() {
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_R) == GLFW_PRESS) {
        resetSimulation();
    }
}

RefractionSimulation::~RefractionSimulation() {
    glDeleteVertexArrays(1, &interfaceVAO);
    glDeleteBuffers(1, &interfaceVBO);

          // Clean up base class resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
void RefractionSimulation::updateRays() {
    // Clear existing rays
    incidentRays.clear();
    refractedRays.clear();
    
    // Create new incident ray with updated angle
    LightRay newRay;
    newRay.origin = glm::vec2(0.0f, 5.0f);  // Same origin point as in resetSimulation
    
    // Convert angle to radians and calculate direction vector
    float angleRadians = glm::radians(incidentAngle);
    newRay.direction = glm::vec2(sin(angleRadians), -cos(angleRadians));
    newRay.intensity = 1.0f;
    
    // Add the new ray to incident rays vector
    incidentRays.push_back(newRay);
    
    // Recalculate refraction for the updated incident ray
    calculateRefraction();
}