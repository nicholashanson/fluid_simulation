/*
    all the // start ... and // end ... comments are used for profiling
*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <fs/fs.hpp>
#include <fs/lbm/js.hpp>

#include <gl.hpp>

#include <cmath>

const int target_fps = 6;
const auto frame_duration = std::chrono::milliseconds( 1000 / target_fps );

const int opencv_colormaps[] = {
    cv::COLORMAP_PLASMA,
    cv::COLORMAP_VIRIDIS,
    cv::COLORMAP_JET,
    cv::COLORMAP_INFERNO,
    cv::COLORMAP_MAGMA,
    cv::COLORMAP_HOT,
    cv::COLORMAP_COOL,
    cv::COLORMAP_SPRING,
    cv::COLORMAP_SUMMER,
    cv::COLORMAP_AUTUMN,
    cv::COLORMAP_WINTER,
    cv::COLORMAP_RAINBOW,
    cv::COLORMAP_OCEAN,
    cv::COLORMAP_PARULA,
};

const char* colormaps[] = {
    "Plasma",
    "Viridis",
    "Jet",
    "Inferno",
    "Magma",
    "Hot",
    "Cool",
    "Spring",
    "Summer",
    "Autumn",
    "Winter",
    "Rainbow",
    "Ocean",
    "Parula",
};

int selected_colormap = 0;

size_t frame_counter = 0;
const size_t save_interval = 10;

const size_t steps_per_frame = 20;

int main() {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> D2Q9_grid( fs::lbm::D2Q9_states );

    sim::grid<std::vector<double>, fs::property_view> property_grid( fs::lbm::property_states );

    fs::lbm::initialize_grid( D2Q9_grid );
	
    fs::lbm::obstacle_coords = fs::lbm::get_airfoil_coords();

    std::vector<unsigned char> barrier( fs::settings::ydim * fs::settings::xdim, 0 ); 

	for ( auto xy : fs::lbm::obstacle_coords ) {
	    barrier[ xy.second + xy.first * fs::settings::xdim ] = 1;
    }

#ifdef GPU

    // void* cs_state = fs::dpcxx::lbm::init_cs( D2Q9_grid, barrier, 0.05 );
#endif

    // initialize GLFW and OpenGL context
    GLFWwindow* window = initialize_window();

    unsigned int shader_program = setup_openGL();

    std::vector<float> barrier_vertices = app::obstacle_to_vertex_data<fs::settings::ydim,fs::settings::xdim>( barrier );

    projection( shader_program ); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 130" );
    
    std::vector<float> vertices;

    // setup OpenGL buffers
    unsigned int VAO, VBO;
    app::setup_grid_buffers( vertices, VAO, VBO );

    bool simulation_running = false;

    // render loop
    while ( !glfwWindowShouldClose( window ) ) {

        auto frame_start = std::chrono::steady_clock::now();

        // input handling ( close on escape key )
        if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
            glfwSetWindowShouldClose( window, true );

        // start imgui set up

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos( ImVec2( 0, 0 ), ImGuiCond_Always );

        ImGui::Begin( "Simulation Controls" );

        if ( ImGui::Button( simulation_running ? "Stop Simulation" : "Start Simulation" ) ) {

            simulation_running = !simulation_running;
        }

        ImGui::Combo( "Colormap", &selected_colormap, colormaps, IM_ARRAYSIZE( colormaps ) );

        ImGui::End();

        // end imgui set up

        if ( simulation_running ) {

            // start grid buffer setup

            app::setup_grid_buffers( vertices, VAO, VBO );

            // end grid buffer setup

            // start boundary setting

            fs::lbm::set_boundaries( D2Q9_grid );

            // end boundary setting

#ifndef GPU

            // start collide and stream

            fs::lbm::collide_and_stream_tbb( D2Q9_grid.get_data_handle(), barrier.data(), steps_per_frame );

            // end collide and stream
#else
            // start collide and stream

            fs::dpcxx::lbm::collide_and_stream( D2Q9_grid, barrier.data(), steps_per_frame );

            // end collide and stream
#endif

            // start vertex calculation

#ifdef GPU

            fs::lbm::calculate_property_v_tbb( D2Q9_grid.get_data_handle(), property_grid.get_data_handle(), fs::lbm::calculate_u_x );

            vertices = fs::dpcxx::lbm::grid_to_vertex_data( property_grid );
#else
            vertices = app::property_grid_to_vertex_data_cv_tbb_copy( D2Q9_grid, fs::lbm::property_states, fs::lbm::calculate_u_x, selected_colormap );
#endif

            vertices.insert( vertices.end(), barrier_vertices.begin(), barrier_vertices.end() );

            // end vertex calculation

            // start render set up

            GLint vbo_size;
            glBindBuffer( GL_ARRAY_BUFFER, VBO );
            glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vbo_size );
        
            // if the VBO is not large enough, reallocate it
            if ( vertices.size() * sizeof( float ) > vbo_size ) {
                glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_DYNAMIC_DRAW );
            } else {
                glBufferSubData( GL_ARRAY_BUFFER, 0, vertices.size() * sizeof( float ), vertices.data() );
            }

            glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT );

            // end render set up

            // start render

            app::render_grid( VAO, vertices.size() );

            // end render
        
            frame_counter++;
        }

        // start imgui render

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        // end imgui render

        int display_w;
        int display_h;
        glfwGetFramebufferSize( window, &display_w, &display_h );

        glViewport( 0, 0, display_w, display_h );

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

#ifdef GPU
    // fs::dpcxx::lbm::terminate_cs_c( cs_state );
#endif

    return 0;
}


