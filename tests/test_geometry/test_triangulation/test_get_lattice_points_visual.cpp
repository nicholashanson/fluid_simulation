#include <gtest/gtest.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <geometry/triangulation.hpp>

TEST( VisualGeometryTests, GetLatticPointsVisual ) {
    std::vector<std::pair<double,double>> lattice_points = geometry::get_lattice_points( 0.0, 2.0, 0.0, 2.0, 50, 50 );

    GLFWwindow* window = app::initialize_window_for_test( "Lattic Points" );
    ASSERT_NE( window, nullptr );
    GLuint VAO, VBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    auto point_shader_program = app::get_point_shader_program();
    app::get_projection( point_shader_program, 0.0, 2.0, 0.0, 2.0 );
    app::identity_view( point_shader_program );
    app::identity_model( point_shader_program );  

    std::vector<glm::vec2> vertices;
    for ( auto& point : lattice_points ) {
    	vertices.push_back( glm::vec2( point.first, point.second ) );
    }
    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( glm::vec2 ), ( void* )0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    double start_time = glfwGetTime();
    while ( !glfwWindowShouldClose( window ) && glfwGetTime() - start_time < 3.0 ) {
        glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glBindVertexArray( VAO );

        glUseProgram( point_shader_program );
        app::get_projection( point_shader_program, 0.0, 2.0, 0.0, 2.0 );
        app::identity_view( point_shader_program );
        app::identity_model( point_shader_program );

        glPointSize( 2.0f );
        glDrawArrays( GL_POINTS, 0, vertices.size() );
        glBindVertexArray( 0 );
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &VBO );
    glDeleteVertexArrays( 1, &VAO );
    glfwDestroyWindow( window );
    glfwTerminate();
}
