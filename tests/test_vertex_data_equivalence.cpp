#include <gtest/gtest.h>

#include <fs/fs.hpp>
#include <fs/lbm/js.hpp>
#include <settings.hpp>
#include <grid.hpp>

#include <vector>

TEST( LBMTests, VertexDataEquivalence ) {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> lbm_grid( fs::lbm::D2Q9_states );

    std::vector<float> vertices_cv;
    std::vector<float> vertices_cv_tbb;

    vertices_cv = app::property_grid_to_vertex_data_cv( lbm_grid, fs::lbm::property_states, fs::lbm::calculate_u_x );
    vertices_cv_tbb = app::property_grid_to_vertex_data_cv_tbb( lbm_grid, fs::lbm::property_states, fs::lbm::calculate_u_x );

    ASSERT_EQ( vertices_cv, vertices_cv_tbb ) << "vertex data is inconsistent";
} 