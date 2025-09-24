#include <gtest/gtest.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>
#include <numeric>

#ifndef M_PI
#define M_PI ( 4 * std::atan( 1 ) )
#endif

#include <geometry/polygon.hpp>

TEST( GeometryVisualTest, BSpline_Evaluate ) {
	auto spline = geometry::b_spline<double>( 
		{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
	std::vector<glm::vec2> vertices;
	for ( double t = 0.0; t <= 1.0; t += 0.01 ) {
		auto point = geometry::de_boor( spline, t );
		vertices.emplace_back( point.first, point.second );
	}
	const std::size_t num_of_points = vertices.size();
	for ( auto& point : spline.control_points ) {
		vertices.emplace_back( point.first, point.second );
	}
    GLFWwindow* window = app::initialize_window_for_test( "B Spline" );
    ASSERT_NE( window, nullptr );
    GLuint vao, vbo;
    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );

    unsigned int triangle_shader_program = app::get_triangle_shader_program();
    app::test_projection( triangle_shader_program );
    app::identity_view( triangle_shader_program );
    app::identity_model( triangle_shader_program );
    auto point_shader_program = app::get_point_shader_program();
    app::test_projection( point_shader_program );
    app::identity_view( point_shader_program );
    app::identity_model( point_shader_program );  
    auto text_shader_program = app::get_text_shader_program();

    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( glm::vec2 ), ( void* )0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    double start_time = glfwGetTime();
    while ( !glfwWindowShouldClose( window ) && glfwGetTime() - start_time < 3.0 ) {
        glClearColor( 0.1f, 0.1f, 0.2f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glBindVertexArray( vao );
        glUseProgram( triangle_shader_program );
        glDrawArrays( GL_LINE_STRIP, 0, num_of_points );
        glUseProgram( point_shader_program );
        app::test_projection( point_shader_program );
        app::identity_view( point_shader_program );
        app::identity_model( point_shader_program );
        glPointSize( 10.0f );
        glDrawArrays( GL_POINTS, num_of_points, spline.control_points.size() );
        glBindVertexArray( 0 );
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &vbo );
    glDeleteVertexArrays( 1, &vao );
    glfwDestroyWindow( window );
    glfwTerminate();
}