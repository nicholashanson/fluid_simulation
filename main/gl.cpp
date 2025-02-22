#include <gl.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// callback function to handle window resizing
void framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
    glViewport( 0, 0, width, height );
}

GLFWwindow* initialize_window() {

    // initialize GLFW
    if ( !glfwInit() ) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // configure GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    // create a GLFW window
    GLFWwindow* window = glfwCreateWindow( fs::settings::window_width, fs::settings::window_height, "LGA Simulation", nullptr, nullptr );

    if ( !window ) {
        glfwTerminate();
        throw std::runtime_error( "Failed to create GLFW window" );
    }

    glfwMakeContextCurrent( window );

    // initialize GLAD
    if ( !gladLoadGLLoader( ( GLADloadproc)glfwGetProcAddress ) ) {
        glfwDestroyWindow( window );
        glfwTerminate();
        throw std::runtime_error( "Failed to initialize GLAD" );
    }

    // set the viewport and configure callback for window resizing
    glViewport( 0, 0, fs::settings::window_width, fs::settings::window_height );

    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    return window;
}

unsigned int setup_openGL() {

    std::string vertex_path = "../shaders/grid_vertex.glsl";
    std::string fragment_path = "../shaders/grid_fragment.glsl";

    // load and compile shaders
    unsigned int shader_program = app::setup_shaders( vertex_path, fragment_path );
    glUseProgram( shader_program );

    return shader_program;
}

void projection( int shader_program ) {

    float aspect_ratio = (float)fs::settings::window_width / (float)fs::settings::window_height;

	float left = -1.0f;
	float right = 1.0f;
	float bottom = 1.0f - ( 2.0f / aspect_ratio );
	float top = 1.0f;

	glm::mat4 projection = glm::ortho( left, right, bottom, top );

    GLuint projectionLoc = glGetUniformLocation( shader_program, "projection" );
    glUniformMatrix4fv( projectionLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
}