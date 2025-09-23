#include <gtest/gtest.h>

#include "test_constants.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <geometry/polygon.hpp>

TEST( VisualGeometryTests, DistanceToSingleSegment1 ) {

    std::vector<size_t> boundary_nodes = { 0, 1 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( 
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

    auto text_shader_program = app::get_text_shader_program();

    // text rendering parameters
    float padding = 10.0f;
    float approx_char_width = 8.0f;
    float x = 10.0f;
    float y = 10.0f;

    GLuint text_VAO, text_VBO;
    app::init_text_vao_vbo( text_VAO, text_VBO );

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

        glUseProgram( text_shader_program );
        app::test_projection( text_shader_program );
        app::identity_view( text_shader_program );
        app::identity_model( text_shader_program );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDisable( GL_DEPTH_TEST );

        app::render_text( "Distance from point to segment: " + std::to_string( dist ), x, y, text_shader_program, text_VAO, text_VBO, 500, 500 );

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

TEST( VisualGeometryTests, DistanceToSingleSegment2 ) {

    std::vector<size_t> boundary_nodes = { 0, 3 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( 
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
