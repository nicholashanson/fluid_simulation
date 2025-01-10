#include <fs.hpp>

namespace fs {

    std::vector<velocity> cell_states( settings::grid_height * settings::grid_width, { 0, 0 } );

    std::vector<double> cell_states_( settings::grid_height * settings::grid_width * 9, 0.0 );

    std::vector<double> vec_mag_states( settings::grid_height * settings::grid_width );

}
