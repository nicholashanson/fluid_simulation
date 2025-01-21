#include <performance_profile.hpp>

#include <fs.hpp>
#include <lbm/collision_step_dpcpp.hpp>

using namespace fs::lbm;

struct f : profiler::fixture<20,1> {

    f() : gd( cell_states_ ) {
        initialize_grid( gd );
    }

    fs::grid<std::vector<double>, fs::lb_grid> gd;
};

PROFILE( dpcpp, collision_step, f, {

    set_tunnel_inlet( fx.gd, 0.01, 0.0 );
    streaming_step( fx.gd, cell_states_ );
    flush_tunnel_walls( fx.gd );

    START_TIMER
    collision_step_dpcpp( fx.gd, cell_states_, 3.0 );
    END_TIMER
})

GENERATE_GRAPH( collision_step )


