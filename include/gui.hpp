#ifndef GUI_HPP
#define GUI_HPP

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>
#include <functional>
#include <span>

namespace app {

    extern std::map<int, std::function<double( std::span<double> )>> properties;

    extern int selected_colormap;

    extern const int opencv_colormaps[];
    
    extern const char* colormaps[];

    void init_imgui( GLFWwindow* window );

    void setup_imgui( bool& simulation_running );
}

#endif