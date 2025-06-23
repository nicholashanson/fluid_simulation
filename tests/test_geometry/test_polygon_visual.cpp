#include <gtest/gtest.h>

#include "test_constants.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <fs/fvm/triangulation.hpp>
#include <geometry/polygon.hpp>

TEST( VisualGeometryTests, DistanceToSingleSegment1 ) {

    std::vector<size_t> boundary_nodes = { 0, 1 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::distance_to_polygon_single_segment( 
        p,                      // the point 
        test::square_points,    // the points that deine the polygon
        boundary_nodes          // the nodes that define the single segement
    );

    GLFWwindow* window = app::initialize_window_for_test( "Distance from Point to Segment" );
    ASSERT_NE( window, nullptr );

    std::vector<glm::vec2> square; 
    for ( auto& point : test::square_points ) {
        square.emplace_back( glm::vec2( point.first, point.second ) ); 
    }

    std::vector<glm::vec2> boundary_node_vertices;
    for ( auto node: boundary_nodes ) {
        boundary_node_vertices.emplace_back( glm::vec2( test::square_points[ node ].first,
                                                        test::square_points[ node ].second )
        ); 
    }

    std::vector<glm::vec2> vertices;
    vertices.insert( vertices.end(), square.begin(), square.end() );
    vertices.insert( vertices.end(), boundary_node_vertices.begin(), boundary_node_vertices.end() );
    vertices.emplace_back( glm::vec2( p.first, p.second ) );

    GLuint VAO, VBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    unsigned int triangle_shader_program = app::get_triangle_shader_program();
    app::get_projection( triangle_shader_program, -20.0, 20.0, -20.0, 20.0 );
    app::identity_view( triangle_shader_program );
    app::identity_model( triangle_shader_program );

    auto point_shader_program = app::get_point_shader_program();

    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );

    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( glm::vec2 ), ( void* )0 );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    double start_time = glfwGetTime();
    while ( !glfwWindowShouldClose( window ) && glfwGetTime() - start_time < 3.0 ) {
        glClearColor( 0.1f, 0.1f, 0.2f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( triangle_shader_program );
        app::get_projection( triangle_shader_program, -20.0, 20.0, -20.0, 20.0 );
        app::identity_view( triangle_shader_program );
        app::identity_model( triangle_shader_program );

        glBindVertexArray( VAO );
        glDrawArrays( GL_LINE_LOOP, 0, 4 ); 

        glUseProgram( point_shader_program);
        app::get_projection( point_shader_program, -20.0, 20.0, -20.0, 20.0 );
        app::identity_view( point_shader_program );
        app::identity_model( point_shader_program );

        glPointSize( 10.0f );
        glDrawArrays( GL_POINTS, 4, 3 );

        glBindVertexArray( 0 );
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &VBO );
    glDeleteVertexArrays( 1, &VAO );
    glfwDestroyWindow( window );
    glfwTerminate();
}