#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

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

std::mutex D2Q9_mutex;
std::condition_variable D2Q9_cv;
std::atomic<bool> D2Q9_ready( false );
std::atomic<bool> should_exit( false );

const int target_fps = 6;
const auto frame_duration = std::chrono::milliseconds( 1000 / target_fps );

const int opencv_colormaps[] = {
    cv::COLORMAP_PLASMA,  
    cv::COLORMAP_VIRIDIS,  
    cv::COLORMAP_JET,     
};

void compute_next_D2Q9_state(
    sim::grid<std::vector<double>, fs::lbm::D2Q9_view>& D2Q9_grid,
    std::vector<unsigned char>& barrier,
    size_t steps_per_frame
) {
    while ( !should_exit ) {

        {
            std::unique_lock<std::mutex> lock( D2Q9_mutex );

            D2Q9_cv.wait( lock, [] { return !D2Q9_ready.load() || should_exit.load(); } );

            if ( should_exit ) break;

            fs::lbm::set_boundaries( D2Q9_grid );
            fs::dpcxx::lbm::collide_and_stream( D2Q9_grid, barrier.data(), steps_per_frame );
            
            D2Q9_ready = true;
            
            lock.unlock();
        }

        D2Q9_cv.notify_one();
    }
}

const char* colormaps[] = { "Plasma", "Viridis", "Jet" };
int selected_colormap = 0;

size_t frame_counter = 0;
const size_t save_interval = 10;

const size_t steps_per_frame = 20;

int main() {

    void* cs_state = fs::dpcxx::lbm::init_cs_c( fs::settings::ydim, fs::settings::xdim, 0.05 );

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> D2Q9_grid( fs::lbm::D2Q9_states );

    sim::grid<std::vector<double>, fs::property_view> property_grid( fs::lbm::property_states );

    fs::lbm::initialize_grid( D2Q9_grid );
	
    fs::lbm::obstacle_coords = fs::lbm::get_obstacle_coords();

    std::vector<unsigned char> barrier( fs::settings::ydim * fs::settings::xdim, 0 ); 

	for ( auto xy : fs::lbm::obstacle_coords ) {
	    barrier[ xy.second + xy.first * fs::settings::xdim ] = 1;
    }

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

    std::thread compute_thread( compute_next_D2Q9_state, std::ref( D2Q9_grid ), std::ref( barrier ), steps_per_frame );

    // render loop
    while ( !glfwWindowShouldClose( window ) ) {

        auto frame_start = std::chrono::steady_clock::now();

        // input handling ( close on escape key )
        if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
            glfwSetWindowShouldClose( window, true );

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos( ImVec2( 0, 0 ), ImGuiCond_Always );

        ImGui::Begin( "Simulation Controls" );

        if ( ImGui::Button( simulation_running ? "Stop Simulation" : "Start Simulation" ) ) {

            simulation_running = !simulation_running;

            if ( simulation_running ) {
            
                D2Q9_cv.notify_one();
            }
        }

        ImGui::Combo( "Colormap", &selected_colormap, colormaps, IM_ARRAYSIZE( colormaps ) );

        ImGui::End();

        if ( simulation_running ) {

            std::unique_lock<std::mutex> lock( D2Q9_mutex );
            
            D2Q9_cv.wait( lock, []{ return D2Q9_ready.load(); } );

            app::setup_grid_buffers( vertices, VAO, VBO );

            vertices = app::property_grid_to_vertex_data( D2Q9_grid, fs::lbm::property_states, fs::lbm::calculate_u_x );
            vertices.insert( vertices.end(), barrier_vertices.begin(), barrier_vertices.end() );

            D2Q9_ready = false;
            lock.unlock();
            D2Q9_cv.notify_one();

            GLint vbo_size;
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vbo_size);
        
            // if the VBO is not large enough, reallocate it
            if ( vertices.size() * sizeof( float ) > vbo_size ) {
                glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_DYNAMIC_DRAW );
            } else {
                glBufferSubData( GL_ARRAY_BUFFER, 0, vertices.size() * sizeof( float ), vertices.data() );
            }

            glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT );

            app::render_grid( VAO, vertices.size() );
        
            frame_counter++;
        }

        ImGui::Render();

        int display_w;
        int display_h;
        glfwGetFramebufferSize( window, &display_w, &display_h );

        glViewport( 0, 0, display_w, display_h );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        // swap buffers and poll events
        glfwSwapBuffers( window );
        glfwPollEvents();

        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;

        if ( elapsed < frame_duration ) 
            std::this_thread::sleep_for( frame_duration - elapsed );
    }

    // cleanup
    should_exit = true;
    D2Q9_cv.notify_one();
    compute_thread.join();

    glDeleteVertexArrays( 1, &VAO );
    glDeleteBuffers( 1, &VBO );
    glDeleteProgram( shader_program );

    glfwTerminate();

    return 0;
}


