#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <fs.hpp>

#include <chrono>
#include <thread>

const int target_fps = 4;
const auto frame_duration = std::chrono::milliseconds( 1000 / target_fps );

// callback function to handle window resizing
void framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
    glViewport( 0, 0, width, height );
}

int main() {

    // initialize GLFW
    if ( !glfwInit() ) {

        std::cerr << "failed to initialize GLFW" << std::endl;

        return -1;
    }

    // configure GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    // create a GLFW window
    GLFWwindow* window = glfwCreateWindow( fs::settings::window_width, fs::settings::window_height, "LGA Simulation", nullptr, nullptr );

    if ( !window ) {

        std::cerr << "failed to create GLFW window" << std::endl;

        glfwTerminate();

        return -1;

    }

    glfwMakeContextCurrent( window );

    // initialize GLAD
    if ( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) ) {

        std::cerr << "failed to initialize GLAD" << std::endl;

        return -1;
    }

    // set the viewport and configure callback for window resizing
    glViewport( 0, 0, fs::settings::window_width, fs::settings::window_height );

    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    std::string vertex_path = "shaders/grid_vertex.glsl";
    std::string fragment_path = "shaders/grid_fragment.glsl";

    // load and compile shaders
    unsigned int shader_program = fs::setup_shaders( vertex_path, fragment_path );
    glUseProgram( shader_program );

    fs::grid<std::vector<double>, fs::lb_grid> gd( fs::cell_states_ );

    fs::lbm::initialize_grid( gd );
    fs::lbm::set_tunnel_inlet( gd, 0.01, 0.0 );

    // convert grid to vertex data
    std::vector<float> vertices = fs::lb_to_vertex_data( gd, fs::vec_mag_states );

    // setup OpenGL buffers
    unsigned int VAO, VBO;
    fs::setup_grid_buffers( vertices, VAO, VBO );

    // render loop
    while ( !glfwWindowShouldClose( window ) ) {

        auto frame_start = std::chrono::steady_clock::now();

        // input handling ( close on escape key )
        if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
            glfwSetWindowShouldClose( window, true );

        vertices = fs::lb_to_vertex_data( gd, fs::vec_mag_states );
        fs::setup_grid_buffers( vertices, VAO, VBO );

        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferSubData( GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data() );

        // clear the screen
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        // render the grid
        fs::render_grid( VAO, vertices.size() );

        gd = fs::lbm::streaming_step( gd, fs::cell_states_ );
        fs::lbm::flush_tunnel_walls( gd );
        fs::lbm::apply_obstacle_boundary( gd );
        gd = fs::lbm::collision_step_dpcpp( gd, fs::cell_states_, 3.0 );
        fs::lbm::set_tunnel_inlet( gd, 0.01, 0.0 );

        // swap buffers and poll events
        glfwSwapBuffers( window );
        glfwPollEvents();

        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;

        if ( elapsed < frame_duration )
            std::this_thread::sleep_for( frame_duration - elapsed );
    }

    // cleanup
    glDeleteVertexArrays( 1, &VAO );
    glDeleteBuffers( 1, &VBO );
    glDeleteProgram( shader_program );

    glfwTerminate();

    return 0;
}

