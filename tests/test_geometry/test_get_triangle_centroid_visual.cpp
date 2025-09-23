#include <gtest/gtest.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <geometry/triangle.hpp>

TEST( VisualGeometryTest, GetTriangleCentroid ) {
    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    auto triangle_centroid = geometry::get_triangle_centroid( p, q, r );

    GLFWwindow* window = app::initialize_window_for_test( "Triangle Centroid" );
    ASSERT_NE( window, nullptr );

    std::vector<glm::vec2> triangle = {
        glm::vec2( p.first, p.second ),
        glm::vec2( q.first, q.second ),
        glm::vec2( r.first, r.second )
    };

    auto midpoint = []( std::pair<double,double> a, std::pair<double,double > b ) {
        return std::make_pair( 0.5 * ( a.first + b.first ), 0.5 * ( a.second + b.second ) );
    };

    auto m_1 = midpoint( q, r );
    auto m_2 = midpoint( p, r );
    auto m_3 = midpoint( p, q );

    std::vector<glm::vec2> medians = {
        glm::vec2( p.first, p.second ), glm::vec2( m_1.first, m_1.second ),
        glm::vec2( q.first, q.second ), glm::vec2( m_2.first, m_2.second ),
        glm::vec2( r.first, r.second ), glm::vec2( m_3.first, m_3.second )
    };

    glm::vec2 centroid_point( triangle_centroid.first, triangle_centroid.second );

    GLuint VAO, VBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    unsigned int triangle_shader_program = app::get_triangle_shader_program();
    app::get_projection( triangle_shader_program, -2.0, 2.0, -2.0, 2.0 );
    app::identity_view( triangle_shader_program );
    app::identity_model( triangle_shader_program );

    auto point_shader_program = app::get_point_shader_program();

    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );

    std::vector<glm::vec2> vertices;
    vertices.insert( vertices.end(), triangle.begin(), triangle.end() );
    vertices.insert( vertices.end(), medians.begin(), medians.end() );
    vertices.push_back( centroid_point );

    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( glm::vec2 ), ( void* )0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    double start_time = glfwGetTime();
    while ( !glfwWindowShouldClose(window) && glfwGetTime() - start_time < 3.0 ) {
        glClearColor( 0.1f, 0.1f, 0.2f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( triangle_shader_program );
        app::get_projection( triangle_shader_program, -2.0, 2.0, -2.0, 2.0 );
        app::identity_view( triangle_shader_program );
        app::identity_model( triangle_shader_program );

        glBindVertexArray( VAO );
        glDrawArrays( GL_LINE_LOOP, 0, 3 ); 
        glDrawArrays( GL_LINES, 3, 6 ); 

        glUseProgram( point_shader_program);
        app::get_projection( point_shader_program, -2.0, 2.0, -2.0, 2.0 );
        app::identity_view( point_shader_program );
        app::identity_model( point_shader_program );

        glPointSize( 10.0f );
        glDrawArrays( GL_POINTS, 9, 1 );

        glBindVertexArray( 0 );
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &VBO );
    glDeleteVertexArrays( 1, &VAO );
    glfwDestroyWindow( window );
    glfwTerminate();
}