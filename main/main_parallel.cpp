/*
    all the // start ... and // end ... comments are used for profiling
*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tbb/tbb.h>

#include <opencv2/opencv.hpp>

#include <iostream>

#include <chrono>
#include <thread>

#include <vector>
#include <map>

#include <fs/fs.hpp>
#include <fs/lbm/js.hpp>

#include <gl.hpp>
#include <gui.hpp>

#include <cmath>

const int target_fps = 60;

tbb::task_group group;

const auto frame_duration = std::chrono::milliseconds( 1000 / target_fps );

size_t frame_counter = 0;
const size_t save_interval = 10;

const size_t steps_per_frame = 20;

int main() {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> D2Q9_grid( fs::lbm::D2Q9_states );

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> D2Q9_grid_copy( fs::lbm::D2Q9_states );

    sim::grid<std::vector<double>, fs::property_view> property_grid( fs::lbm::property_states );

    fs::lbm::initialize_grid( D2Q9_grid );
	
    fs::lbm::obstacle_coords = fs::lbm::get_airfoil_coords_aoa( 0.04, 0.4, 0.12, 0.0 );

    std::vector<unsigned char> barrier( fs::settings::ydim * fs::settings::xdim, 0 ); 

	for ( auto xy : fs::lbm::obstacle_coords ) {
	    barrier[ xy.second + xy.first * fs::settings::xdim ] = 1;
    }

#ifdef SF

    void* cs_state = fs::dpcxx::lbm::init_cs( D2Q9_grid, barrier, 0.005 );
#endif

    // initialize GLFW and OpenGL context
    GLFWwindow* window = app::initialize_window();

    unsigned int shader_program = app::setup_openGL();

    std::vector<float> barrier_vertices = app::obstacle_to_vertex_data<fs::settings::ydim,fs::settings::xdim>( barrier );

    app::projection( shader_program ); 
    
    std::vector<float> vertices;

    app::gui::init_imgui( window );

    // setup OpenGL buffers
    unsigned int VAO, VBO;
    app::init_grid_buffers( vertices, VAO, VBO );

    bool simulation_running = false;

    // render loop
    while ( !glfwWindowShouldClose( window ) ) {

        // start loop

        auto frame_start = std::chrono::steady_clock::now();

        // input handling ( close on escape key )
        if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
            glfwSetWindowShouldClose( window, true );

        // start imgui set up

        app::gui::setup_imgui( simulation_running );

        // end imgui set up

        if ( simulation_running ) {

            
#ifndef SF

            // start boundary setting

            fs::lbm::set_boundaries( D2Q9_grid );

            // end boundary setting
#endif

            // start grid copy

            D2Q9_grid_copy = D2Q9_grid;

            // end grid copy

            // start parallel tasks

            group.run( [&]() {

#ifndef GPU

                // start collide and stream
            
                fs::lbm::collide_and_stream_tbb( D2Q9_grid.get_data_handle(), barrier.data(), steps_per_frame );
            
                // end collide and stream
#else // GPU

                // start collide and stream       
#ifdef SF

                fs::dpcxx::lbm::stateful_collide_and_stream_c( cs_state, steps_per_frame );
#else // SF

                fs::dpcxx::lbm::collide_and_stream( D2Q9_grid, barrier.data(), steps_per_frame );
#endif // SF

                // end collide and stream
#endif // GPU
            });

            group.run( [&]() {

                // start grid buffer set up

                app::refresh_grid_buffers( vertices, VBO );

                // end grid buffer set up

                // start property calculation
            
                // selected_property = curl
                if ( std::strcmp( app::gui::properties[ app::gui::selected_property ], "curl" ) == 0 ) {
                    fs::lbm::calculate_curl_v_tbb( D2Q9_grid_copy.get_data_handle(), property_grid.get_data_handle() );
                } else {
                    fs::lbm::calculate_property_v_tbb( D2Q9_grid_copy.get_data_handle(), property_grid.get_data_handle(),
                                                       app::gui::physical_properties[ app::gui::selected_property ] );    
                }

                // end property calculation

                // start vertex calculation

                vertices = app::property_to_vertex_data( property_grid.get_data_handle(), 
                                                         app::gui::opencv_colormaps[ app::gui::selected_colormap ] );

            
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

                // start imgui render

                ImGui::Render();

                ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

                // end imgui render
            });

            group.wait();

            // end parallel tasks
        } else {

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        }

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

#ifdef SF

    fs::dpcxx::lbm::terminate_cs_c( cs_state );
#endif

    return 0;
}


