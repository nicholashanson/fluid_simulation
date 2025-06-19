#include <gtest/gtest.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl.hpp>
#include <shader.hpp>

#include <cmath>

#include <fs/fvm/triangulation.hpp>
#include <geometry/circle.hpp>

TEST( VisualGeometryTests, InCircle ) {

    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    std::pair<double,double> a(  5.0, 5.0 );

    auto expected_outside = geometry::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_outside, fs::fvm::in_circle::OUTSIDE );

    GLFWwindow* window = app::initialize_window_for_test();
    ASSERT_NE(window, nullptr);

    auto triangle_center = fs::fvm::get_triangle_circumcenter( p, q, r );
    glm::vec2 circle_center( triangle_center.first, triangle_center.second );
    auto circle_vertices = app::generate_circle_vertices( circle_center, 1.0 );

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    unsigned int shader_program = app::setup_openGL_for_test();
    app::test_projection(shader_program);
    app::identity_view(shader_program);
    app::identity_model(shader_program);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, circle_vertices.size() * sizeof(glm::vec2), circle_vertices.data(), GL_STATIC_DRAW);

    // Setup vertex attribute for position at location 0 (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Your drawing loop
    double start_time = glfwGetTime();
    while (!glfwWindowShouldClose(window) && glfwGetTime() - start_time < 3.0) {

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP, 0, circle_vertices.size());
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwDestroyWindow(window);
    glfwTerminate();
}