#ifndef GL_HPP
#define GL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_easy_font.h>

#include <shader.hpp>

#include <string>
#include <vector>
#include <iostream>

#include <settings.hpp>
//#include <fs/fs.hpp>

#ifndef M_PI
#define M_PI ( 4 * std::atan( 1 ) )
#endif

namespace app {

    GLFWwindow* initialize_window();

    GLFWwindow* initialize_window_for_test();

    unsigned int setup_openGL();

    unsigned int setup_openGL_for_test();

    unsigned int setup_openGL_3D();

    void projection( int shader_program );
    
    void test_projection( int shader_program );

    void get_projection( int shader_program, float left, float right, float bottom, float top );

    void projection_3D( int shader_program );

    void projection_3D_orth( int shader_program );

    void view( int shader_program );

    void model( int shader_program );

    std::vector<glm::vec2> generate_circle_vertices( glm::vec2 center, float radius, int segments = 100 );

    void identity_view( int shader_program );

    void identity_model( int shader_program );

    void init_text_vao_vbo(GLuint &VAO, GLuint &VBO);

    void render_text( const std::string& text, float x, float y, unsigned int shader_program, GLuint VAO, GLuint VBO, int window_width, int window_height );

} // namespace app

#endif

