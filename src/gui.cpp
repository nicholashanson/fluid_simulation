#include <gui.hpp>

#include <fs/lbm/property.hpp>

#include <opencv2/opencv.hpp>

namespace app {

    std::map<int, std::function<double( std::span<double> )>> properties = {
        { 0, fs::lbm::calculate_rho },
        { 1, fs::lbm::calculate_u },
        { 2, fs::lbm::calculate_u_x },
        { 3, fs::lbm::calculate_u_y },
    };

    int selected_colormap = 0;

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

    void init_imgui( GLFWwindow* window ) {

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL( window, true );
        ImGui_ImplOpenGL3_Init( "#version 130" );
    }

    void setup_imgui( bool& simulation_running ) {
        
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
    }
}