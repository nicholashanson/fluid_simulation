#include <collision_step_dpcpp.hpp>
#include <global_aliases.hpp>

namespace fs {
namespace lbm {

    template grid<std::vector<double>, lb_grid>
    collision_step_dpcpp (
        grid<std::vector<double>, lb_grid>&,
        std::vector<double>&,
        const double
    );

}
}
