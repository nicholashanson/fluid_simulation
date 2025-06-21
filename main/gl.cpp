#include <gl.hpp>

namespace app {

    // callback function to handle window resizing
    void framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
        glViewport( 0, 0, width, height );
    }

    GLFWwindow* initialize_window() {

        // initialize GLFW
        if ( !glfwInit() ) {
            glfwTerminate();
            throw std::runtime_error( "Failed to create GLFW window" );
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

    GLFWwindow* initialize_window_for_test() {

        // initialize GLFW
        if ( !glfwInit() ) {
            glfwTerminate();
            throw std::runtime_error( "Failed to create GLFW window" );
        }

        // configure GLFW
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

        // create a GLFW window
        GLFWwindow* window = glfwCreateWindow( 500, 500, "Visual Test", nullptr, nullptr );

        if ( !window ) {
            glfwTerminate();
            throw std::runtime_error( "Failed to create GLFW window" );
        }

        int win_width, win_height;
        glfwGetWindowSize(window, &win_width, &win_height);

        glfwMakeContextCurrent( window );

        // initialize GLAD
        if ( !gladLoadGLLoader( ( GLADloadproc)glfwGetProcAddress ) ) {
            glfwDestroyWindow( window );
            glfwTerminate();
            throw std::runtime_error( "Failed to initialize GLAD" );
        }

        glViewport( 0, 0, 500, 500 );

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

    unsigned int setup_openGL_for_test() {

        std::string vertex_path = "../shaders/test_vertex.glsl";
        std::string fragment_path = "../shaders/grid_fragment.glsl";

        // load and compile shaders
        unsigned int shader_program = app::setup_shaders( vertex_path, fragment_path );
        glUseProgram( shader_program );

        return shader_program;
    }

    unsigned int setup_openGL_3D() {

        std::string vertex_path = "../shaders/grid_vertex_3D.glsl";
        std::string fragment_path = "../shaders/grid_fragment.glsl";

        // load and compile shaders
        unsigned int shader_program = app::setup_shaders( vertex_path, fragment_path );
        glUseProgram( shader_program );

        return shader_program;
    }

    void projection( int shader_program ) {

        float aspect_ratio = ( float )fs::settings::window_width / ( float )fs::settings::window_height;

        float left = -1.0f;
        float right = 1.0f;
        float bottom = 1.0f - ( 2.0f / aspect_ratio );
        float top = 1.0f;

        glm::mat4 projection = glm::ortho( left, right, bottom, top );

        GLuint projection_loc = glGetUniformLocation( shader_program, "projection" );
        glUniformMatrix4fv( projection_loc, 1, GL_FALSE, glm::value_ptr( projection ) );
    }

    void test_projection( int shader_program ) {

        float left = -6.0f;
        float right = 6.0f;
        float bottom = -6.0f;
        float top = 6.0f;

        glm::mat4 projection = glm::ortho( left, right, bottom, top );

        GLuint projection_loc = glGetUniformLocation( shader_program, "projection" );
        glUniformMatrix4fv( projection_loc, 1, GL_FALSE, glm::value_ptr( projection ) );
    }

    void projection_3D( int shader_program ) {

        float aspect_ratio = ( float )fs::settings::window_width / ( float )fs::settings::window_height;
        
        float fov = 45.0f;
        float near_plane = 0.1f;
        float far_plane = 100.0f;

        glm::mat4 projection = glm::perspective( glm::radians( fov ), aspect_ratio, near_plane, far_plane );

        GLuint projection_loc = glGetUniformLocation( shader_program, "projection" );
        glUniformMatrix4fv( projection_loc, 1, GL_FALSE, glm::value_ptr( projection ) );
    }

    void projection_3D_orth( int shader_program ) {

        float left = -1000.0f; 
        float right = 1000.0f;
        float bottom = -1000.0f;
        float top = 1000.0f;
        float near_plane = -1000.0f;
        float far_plane = 1000.0f;
    
        glm::mat4 projection = glm::ortho( left, right, bottom, top, near_plane, far_plane );
    
        GLuint projection_loc = glGetUniformLocation( shader_program, "projection" );
        glUniformMatrix4fv( projection_loc, 1, GL_FALSE, glm::value_ptr( projection ) );
    }

    void view( int shader_program ) {

        glm::vec3 camera_pos = glm::vec3( 100.0f, 100.0f, 100.0f );
        glm::vec3 camera_target = glm::vec3( 0.0f, 0.0f, 0.0f );
        glm::vec3 up = glm::vec3( 0.0f, 1.0f, 0.0f );

        glm::mat4 view = glm::lookAt( camera_pos, camera_target, up );

        GLuint view_loc = glGetUniformLocation( shader_program, "view" );
        glUniformMatrix4fv( view_loc, 1, GL_FALSE, glm::value_ptr( view ) );
    }

    void model( int shader_program ) {

        glm::mat4 model = glm::mat4( 1.0f );
        GLuint model_loc = glGetUniformLocation( shader_program, "model" );
        glUniformMatrix4fv( model_loc, 1, GL_FALSE, glm::value_ptr( model ) );
    }

    std::vector<glm::vec2> generate_circle_vertices( glm::vec2 center, float radius, int segments ) {
        std::vector<glm::vec2> vertices;
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * M_PI * i / segments;
            float x = center.x + radius * cos(angle);
            float y = center.y + radius * sin(angle);
            vertices.emplace_back(x, y);
        }
        return vertices;
    }

    void identity_view( int shader_program ) {
        glm::mat4 view = glm::mat4( 1.0f ); 

        GLuint loc = glGetUniformLocation( shader_program, "view" );
        glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( view ) );
    }

    void identity_model( int shader_program ) {
        glm::mat4 model = glm::mat4( 1.0f ); 

        GLuint loc = glGetUniformLocation( shader_program, "model" );
        glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( model ) );
    }

    static unsigned char stb_buffer[ 99999 ]; 

    static unsigned char tri_buffer[ 99999 * 3 / 2 ]; 

    void init_text_vao_vbo( GLuint &VAO, GLuint &VBO ) {
        glGenVertexArrays( 1, &VAO );
        glGenBuffers( 1, &VBO );

        glBindVertexArray( VAO );
        glBindBuffer( GL_ARRAY_BUFFER, VBO );

        glBufferData( GL_ARRAY_BUFFER, sizeof(stb_buffer) * 3 / 2, NULL, GL_DYNAMIC_DRAW ); 

        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 16, ( void* )0 );
        glEnableVertexAttribArray( 0 );

        glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, ( void* )( 3 * sizeof( float ) ) );
        glEnableVertexAttribArray( 1 );

        glBindVertexArray( 0 );
    }

    void render_text( const std::string& text, float x, float y,
                      GLuint shader_program, GLuint VAO, GLuint VBO,
                      int window_width, int window_height ) {

        int num_quads = stb_easy_font_print( x, y, ( char* )text.c_str(), NULL, stb_buffer, sizeof( stb_buffer ) );
        int num_triangles = num_quads * 2;
        int num_vertices = num_triangles * 3;

        for ( int i = 0; i < num_quads; i++ ) {
            unsigned char* v_0 = stb_buffer + i * 4 * 16 + 0 * 16;
            unsigned char* v_1 = stb_buffer + i * 4 * 16 + 1 * 16;
            unsigned char* v_2 = stb_buffer + i * 4 * 16 + 2 * 16;
            unsigned char* v_3 = stb_buffer + i * 4 * 16 + 3 * 16;

            memcpy( tri_buffer + ( i * 6     ) * 16, v_0, 16 );
            memcpy( tri_buffer + ( i * 6 + 1 ) * 16, v_1, 16 );
            memcpy( tri_buffer + ( i * 6 + 2 ) * 16, v_2, 16 );

            memcpy( tri_buffer + ( i * 6 + 3 ) * 16, v_2, 16 );
            memcpy( tri_buffer + ( i * 6 + 4 ) * 16, v_3, 16 );
            memcpy( tri_buffer + ( i * 6 + 5 ) * 16, v_0, 16 );
        }

        glUseProgram( shader_program );

        float ortho[ 4 ][ 4 ] = {
            { 2.0f / window_width, 0, 0, 0 },
            { 0, -2.0f / window_height, 0, 0 },
            { 0, 0, -1.0f, 0 },
            { -1.0f, 1.0f, 0, 1.0f }
        };

        GLint u_projection = glGetUniformLocation( shader_program, "projection" );
        glUniformMatrix4fv( u_projection, 1, GL_FALSE, &ortho[ 0 ][ 0 ] );

        glBindVertexArray( VAO );
        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, num_vertices * 16, tri_buffer, GL_DYNAMIC_DRAW );
        glDrawArrays( GL_TRIANGLES, 0, num_vertices );
        glBindVertexArray( 0 );
    }

} // namespace app

