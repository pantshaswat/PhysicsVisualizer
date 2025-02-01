#define GL_SILENCE_DEPRECATION
#include "config.h"
#include <glm/gtc/matrix_transform.hpp>
#include "projectile_simulation.h"
#include "refraction_simulation.h"
#include "imgui/include/imgui.h"
#include "imgui/include/imgui_impl_glfw.h"
#include "imgui/include/imgui_impl_opengl3.h"
#include <memory>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

enum class SimulationType {
    None,
    Projectile,
    Refraction
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cleanup(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, 
        "Physics Visualizer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD before any OpenGL calls
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set callbacks after window creation and GLAD initialization
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    if (ImGui::CreateContext() == nullptr) {
        std::cerr << "Failed to create ImGui context" << std::endl;
        glfwTerminate();
        return -1;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        cleanup(window);
        return -1;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend" << std::endl;
        cleanup(window);
        return -1;
    }

    // OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Simulation variables
    std::unique_ptr<SimulationBase> currentSimulation;
    SimulationType selectedSimulation = SimulationType::None;
    bool simulationChosen = false;

    // Projection setup
    glm::mat4 projection = glm::ortho(-15.0f, 15.0f, -5.0f, 25.0f, -1.0f, 1.0f);

    // Timing variables
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show simulation selector if no simulation is chosen
        if (!simulationChosen) {
            ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH * 0.5f - 200, SCR_HEIGHT * 0.5f - 100), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
            ImGui::Begin("Select Simulation", nullptr, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            
            ImGui::TextColored(ImVec4(1,1,0,1), "Welcome to Physics Visualizer!");
            ImGui::Text("Please select a simulation to begin:");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Projectile Motion", ImVec2(180, 40))) {
                selectedSimulation = SimulationType::Projectile;
                simulationChosen = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Light Refraction", ImVec2(180, 40))) {
                selectedSimulation = SimulationType::Refraction;
                simulationChosen = true;
            }

            ImGui::End();

            // Initialize chosen simulation

            if (simulationChosen) {
                try {
                    switch (selectedSimulation) {
                        case SimulationType::Projectile:
                            currentSimulation = std::unique_ptr<ProjectileSimulation>(new ProjectileSimulation());;
                            glfwSetWindowTitle(window, "Physics Visualizer - Projectile Motion");
                            break;
                        case SimulationType::Refraction:
                            currentSimulation = std::unique_ptr<RefractionSimulation>(new RefractionSimulation());;
                            glfwSetWindowTitle(window, "Physics Visualizer - Light Refraction");
                            break;
                        default:
                            break;
                    }
                    currentSimulation->init();
                    
                    // Set projection for the new simulation
                    GLuint shaderProgram = currentSimulation->getShaderProgram();
                    glUseProgram(shaderProgram);
                    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
                    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
                } catch (const std::exception& e) {
                    std::cerr << "Simulation initialization failed: " << e.what() << std::endl;
                    simulationChosen = false;
                    currentSimulation.reset();
                }
            }
        } else {
            // Add "Switch Simulation" button in the top-right corner
            ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 120, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(110, 50), ImGuiCond_Always);
            ImGui::Begin("Switch", nullptr, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            
            if (ImGui::Button("Switch Simulation")) {
                simulationChosen = false;
                currentSimulation.reset();
            }
            ImGui::End();

            // Run current simulation
            if (currentSimulation) {
                currentSimulation->handleInput();
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                currentSimulation->render(deltaTime);
            }
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    currentSimulation.reset();
    cleanup(window);
    return 0;
}