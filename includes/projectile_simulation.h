
#pragma once
#include "simulation_base.h"

class ProjectileSimulation : public SimulationBase {
public:
    ~ProjectileSimulation() override;
    void init() override;
    void render(float deltaTime) override;
    void handleInput() override;
    
private:
   struct Projectile {
        glm::vec2 startPosition;      
        glm::vec2 position;
        glm::vec2 velocity;
        float time;
    } projectile;

    // Cannon and target members
    GLuint cannonVAO, cannonVBO;
    GLuint targetVAO, targetVBO;
    float cannonAngle = 45.0f;
    float launchSpeed = 20.0f;
    float targetDistance = 15.0f;
    glm::vec2 targetPosition;

    // Simulation state
    bool simulationRunning = false;
    bool simulationCompleted = false;
    float maxHeight = 0.0f;
    float totalDistance = 0.0f;
    float currentHeight = 0.0f;
    float distanceFromTarget = 0.0f;

    std::vector<glm::vec2> pathPoints;


    void setupProjectileBuffers();
    void setupCannonBuffers();
    void setupTargetBuffers();
    void resetSimulation();
    void updatePhysics(float deltaTime);
   
};