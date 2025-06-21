#include <gtest/gtest.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <fs/fvm/triangulation.hpp>
#include <geometry/circle.hpp>

std::string in_circle_result( fs::fvm::in_circle result ) {
    switch ( result ) {
        case fs::fvm::in_circle::INSIDE:
            return "Point is inside circle";
        case fs::fvm::in_circle::OUTSIDE:
            return "Point is outside circle";
        case fs::fvm::in_circle::ON:
            return "Point is on circle";
    }
    return "";
} 

TEST( VisualGeometryTests, InCircle ) {

    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    std::pair<double,double> a(  5.0, 5.0 );

    auto result = geometry::point_in_circle( p, q, r, a );
    auto message = in_circle_result( result );

    std::cout << message.size() << std::endl;

    GLFWwindow* window = app::initialize_window_for_test();
    ASSERT_NE( window, nullptr );

    auto triangle_center = fs::fvm::get_triangle_circumcenter( p, q, r );
    glm::vec2 circle_center( triangle_center.first, triangle_center.second );
    auto circle_vertices = app::generate_circle_vertices( circle_center, 1.0, 1000 );
    std::vector<glm::vec2> points;
    glm::vec2 point( a.first, a.second ); 
    points.push_back( point );

    std::vector<glm::vec2> vertices;
    vertices.insert( vertices.end(), circle_vertices.begin(), circle_vertices.end() );
    vertices.insert( vertices.end(), points.begin(), points.end() );

    GLuint VAO, VBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    GLuint text_VAO, text_VBO;
    app::init_text_vao_vbo( text_VAO, text_VBO );
    
    // text rendering parameters
    float padding = 10.0f;
    float approx_char_width = 8.0f;
    float x = 10.0f;
    float y = 10.0f;

    unsigned int shader_program = app::setup_openGL_for_test();
    app::test_projection( shader_program );
    app::identity_view( shader_program );
    app::identity_model( shader_program );  

    auto point_shader_program = app::get_point_shader_program();
    app::test_projection( point_shader_program );
    app::identity_view( point_shader_program );
    app::identity_model( point_shader_program );  

    auto text_shader_program = app::get_text_shader_program();

    glBindVertexArray( VAO );

    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec2 ), vertices.data(), GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), ( void* )0 );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    double start_time = glfwGetTime();
    while ( !glfwWindowShouldClose(window) && glfwGetTime() - start_time < 3.0 ) {

        glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glBindVertexArray( VAO );

        glUseProgram( shader_program );
        glDrawArrays( GL_LINE_LOOP, 0, circle_vertices.size() );

        glUseProgram( point_shader_program );
        app::test_projection( point_shader_program );
        app::identity_view( point_shader_program );
        app::identity_model( point_shader_program );
        glPointSize( 10.0f );
        glDrawArrays( GL_POINTS, circle_vertices.size(), points.size() );
        glBindVertexArray( 0 );

        glUseProgram( text_shader_program );
        app::test_projection( text_shader_program );
        app::identity_view( text_shader_program );
        app::identity_model( text_shader_program );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDisable( GL_DEPTH_TEST );

        app::render_text( message, x, y, text_shader_program, text_VAO, text_VBO, 500, 500 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &VBO );
    glDeleteVertexArrays( 1, &VAO );

    glfwDestroyWindow( window );
    glfwTerminate();
}