#define GL_SILENCE_DEPRECATION
#include "config.h"
#include <glm/gtc/matrix_transform.hpp>
#include "projectile_simulation.h"
#include "imgui/include/imgui.h"
#include "imgui/include/imgui_impl_glfw.h"
#include "imgui/include/imgui_impl_opengl3.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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
        "Physics Visualizer - Projectile Motion", NULL, NULL);
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

    // Initialize ImGui (single initialization)
    IMGUI_CHECKVERSION();
    if (ImGui::CreateContext() == nullptr) {
        std::cerr << "Failed to create ImGui context" << std::endl;
        glfwTerminate();
        return -1;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Initialize ImGui backends
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

    // Initialize simulation
    ProjectileSimulation projectileSim;
    try {
        projectileSim.init();
    } catch (const std::exception& e) {
        std::cerr << "Simulation initialization failed: " << e.what() << std::endl;
        cleanup(window);
        return -1;
    }

    // Projection setup
    glm::mat4 projection = glm::ortho(-15.0f, 15.0f, -5.0f, 25.0f, -1.0f, 1.0f);
    GLuint shaderProgram = projectileSim.getShaderProgram();
    glUseProgram(shaderProgram);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

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

        // Simulation update and render
        projectileSim.handleInput();
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        projectileSim.render(deltaTime);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events (only once per frame)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    cleanup(window);
    return 0;
}