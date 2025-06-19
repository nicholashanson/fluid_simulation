#ifndef GL_HPP
#define GL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.hpp>

#include <string>
#include <vector>
#include <iostream>

#include <settings.hpp>
//#include <fs/fs.hpp>

namespace app {

    GLFWwindow* initialize_window();

    GLFWwindow* initialize_window_for_test();

    unsigned int setup_openGL();

    unsigned int setup_openGL_for_test();

    unsigned int setup_openGL_3D();

    void projection( int shader_program );
    
    void test_projection( int shader_program );

    void projection_3D( int shader_program );

    void projection_3D_orth( int shader_program );

    void view( int shader_program );

    void model( int shader_program );

    std::vector<glm::vec2> generate_circle_vertices( glm::vec2 center, float radius, int segments = 100 );

    void identity_view( int shader_program );

    void identity_model( int shader_program );

} // namespace app

#endif

