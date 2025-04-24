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

    namespace gui {

        extern std::map<int, std::function<double( std::span<double> )>> physical_properties;

        extern int selected_property;

        extern int selected_colormap;

        extern const int opencv_colormaps[];
        
        extern const char* colormaps[];

        extern const char* properties[];

        void init_imgui( GLFWwindow* window );

        void setup_imgui( bool& simulation_running );
    
    } // gui

} // app

#endif