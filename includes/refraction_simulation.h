#include "simulation_base.h"
 
class RefractionSimulation : public SimulationBase{
    public:
    ~RefractionSimulation() override;
    void init() override;
    void render(float deltaTime) override;
    void handleInput() override;

    private:
        struct LightRay {
        glm::vec2 origin;
        glm::vec2 direction;
        float intensity;
    };
        // Medium interface (boundary line)
    GLuint interfaceVAO, interfaceVBO;
    std::vector<glm::vec2> interfacePoints;

    // Light rays (using base class VAO, VBO)
    std::vector<LightRay> incidentRays;
    std::vector<LightRay> refractedRays;

        // Simulation parameters
    float incidentAngle = 45.0f;    // in degrees
    float n1 = 1.0f;                // refractive index of medium 1 (air)
    float n2 = 1.33f;               // refractive index of medium 2 (water)
    bool simulationRunning = false;

    float refractionAngle = 0.0f;  // Store the current refraction angle
float reflectionAngle = 0.0f;  // Store the reflection angle for total internal reflection

        // UI state
    bool showCriticalAngle = false;
    float criticalAngle = 0.0f;
    
    void setupInterfaceBuffers();
    void updateRays();
    void calculateRefraction();
    float calculateCriticalAngle();
    void resetSimulation();

    
    

};