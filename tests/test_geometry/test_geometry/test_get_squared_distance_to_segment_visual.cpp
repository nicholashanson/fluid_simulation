#include <gtest/gtest.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gl.hpp>
#include <shader.hpp>
#include <geometry/geometry.hpp>

TEST( VisualGeometryTests, SquaredDistanceToSegmentPointOutside ) {
    std::pair<double, double> p(  0.0, 0.0 );  
    std::pair<double, double> q( 10.0, 0.0 ); 
    std::pair<double, double> r(  5.0, 5.0 ); 

    double dist_2 = geometry::get_squared_distance_to_segment( p.first, p.second, q.first, q.second, r.first, r.second );
    double expected_dist_2 = 25.0;
    EXPECT_DOUBLE_EQ( dist_2, expected_dist_2 );

    GLFWwindow* window = app::initialize_window_for_test( "Squared Distance to Segment" );
    ASSERT_NE( window, nullptr );

    std::vector<glm::vec2> vertices = {
        glm::vec2( p.first, p.second), glm::vec2(q.first, q.second ), 
        glm::vec2( r.first, r.second )  
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    GLuint text_VAO, text_VBO;
    app::init_text_vao_vbo( text_VAO, text_VBO );

    unsigned int triangle_shader_program = app::get_triangle_shader_program();
    app::get_projection( triangle_shader_program, -20.0, 20.0, -20.0, 20.0 );
    app::identity_view( triangle_shader_program );
    app::identity_model( triangle_shader_program );
    auto point_shader_program = app::get_point_shader_program();
    auto text_shader_program = app::get_text_shader_program();

    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( glm::vec2 ), ( void* )0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    std::string distance_label = "Squared distance: 25.0";
    float x = 10.0f;
    float y = 10.0f;

    double start_time = glfwGetTime();
    while (!glfwWindowShouldClose(window) && glfwGetTime() - start_time < 3.0) {
        glClearColor( 0.1f, 0.1f, 0.2f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( triangle_shader_program );
        app::get_projection( triangle_shader_program, -20.0, 20.0, -20.0, 20.0 );
        app::identity_view( triangle_shader_program );
        app::identity_model( triangle_shader_program );

        glBindVertexArray( VAO );
        glDrawArrays( GL_LINES, 0, 2 ); 

        glUseProgram( point_shader_program);
        app::get_projection( point_shader_program, -20.0, 20.0, -20.0, 20.0 );
        app::identity_view( point_shader_program );
        app::identity_model( point_shader_program );

        glPointSize( 10.0f );
        glDrawArrays( GL_POINTS, 2, 1 );

        glUseProgram( text_shader_program );
        app::test_projection( text_shader_program );
        app::identity_view( text_shader_program );
        app::identity_model( text_shader_program );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDisable( GL_DEPTH_TEST );

        app::render_text( distance_label, x, y, text_shader_program, text_VAO, text_VBO, 500, 500 );

        glBindVertexArray( 0 );
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glDeleteBuffers( 1, &VBO );
    glDeleteVertexArrays( 1, &VAO );
    glfwDestroyWindow( window );
    glfwTerminate();
}











    

    

    

