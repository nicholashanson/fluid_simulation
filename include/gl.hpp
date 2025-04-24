#ifndef GL_HPP
#define GL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include <settings.hpp>
#include <fs/fs.hpp>

namespace app {

    GLFWwindow* initialize_window();

    unsigned int setup_openGL();

    unsigned int setup_openGL_3D();

    void projection( int shader_program );

    void projection_3D( int shader_program );

    void projection_3D_orth( int shader_program );

    void view( int shader_program );

    void model( int shader_program );

}

#endif

