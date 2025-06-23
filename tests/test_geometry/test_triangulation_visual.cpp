#include <gtest/gtest.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <fs/fvm/triangulation.hpp>
#include <geometry/circle.hpp>

TEST( VisualGeometryTests, LatticPoints ) {

    std::vector<std::pair<double,double>> lattice_points = fs::fvm::get_lattice_points( 0.0, 2.0, 0.0, 2.0, 50, 50 );

    GLFWwindow* window = app::initialize_window_for_test();
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

TEST( VisualGeometryTests, LatticeTriangles ) {

    fs::fvm::triangle_set triangle_vertex_indices = fs::fvm::get_lattice_triangles( 50, 50 );

    ASSERT_EQ( triangle_vertex_indices.size(), 4802 );

    std::vector<std::pair<double,double>> lattice_points = fs::fvm::get_lattice_points( 0.0, 2.0, 0.0, 2.0, 50, 50 );

    GLFWwindow* window = app::initialize_window_for_test();
    ASSERT_NE( window, nullptr );

    GLuint VAO, VBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    auto triangle_shader_program = app::get_triangle_shader_program();
    app::get_projection( triangle_shader_program, 0.0, 2.0, 0.0, 2.0 );
    app::identity_view( triangle_shader_program );
    app::identity_model( triangle_shader_program );  

    auto vertices = fs::fvm::get_triangle_vertices( triangle_vertex_indices, lattice_points );

    glBindVertexArray( VAO );

    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_DYNAMIC_DRAW );

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), ( void* )0 );
    glEnableVertexAttribArray( 0 );

    double start_time = glfwGetTime();
    while ( !glfwWindowShouldClose( window ) && glfwGetTime() - start_time < 3.0 ) {

        //glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glBindVertexArray( VAO );

        glUseProgram( triangle_shader_program );
        app::get_projection( triangle_shader_program, 0.0, 2.0, 0.0, 2.0 );
        app::identity_view( triangle_shader_program );
        app::identity_model( triangle_shader_program );

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2 );
        glBindVertexArray( 0 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &VBO );
    glDeleteVertexArrays( 1, &VAO );

    glfwDestroyWindow( window );
    glfwTerminate();
}