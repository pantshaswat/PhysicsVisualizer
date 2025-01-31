cmake_minimum_required(VERSION 3.0.0)
project(physics_visualizer VERSION 1.0.0)
cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 11)


find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
# ImGui source files
set(IMGUI_SOURCES
    dependencies/imgui/src/imgui.cpp
    dependencies/imgui/src/imgui_demo.cpp
    dependencies/imgui/src/imgui_draw.cpp
    dependencies/imgui/src/imgui_tables.cpp
    dependencies/imgui/src/imgui_widgets.cpp
    dependencies/imgui/src/imgui_impl_glfw.cpp
    dependencies/imgui/src/imgui_impl_opengl3.cpp
)
add_executable(physics_visualizer
    src/config.h
    src/main.cpp 
    src/glad.c
    src/triangle_mesh.cpp
    src/shader_utils.cpp
    src/simulation_base.cpp
    src/projectile_simulation.cpp
    ${IMGUI_SOURCES}
)


target_include_directories(physics_visualizer 
    PRIVATE
    
    dependencies
    dependencies/imgui/include
    includes
)

target_link_libraries(physics_visualizer
    glfw
    OpenGL::GL 
)