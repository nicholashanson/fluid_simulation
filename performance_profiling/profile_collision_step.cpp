#include <performance_profile.hpp>

#include <fs.hpp>

using namespace fs::lbm;

struct f : profiler::fixture<20,1> {

    f() : gd( cell_states_ ) {
        initialize_grid( gd );
    }

    fs::grid<std::vector<double>, fs::lb_grid> gd;
};

PROFILE_AVG( serial, collision_step, f, {

    set_tunnel_inlet( fx.gd, 0.01, 0.0 );
    streaming_step( fx.gd, cell_states_ );
    flush_tunnel_walls( fx.gd );

    START_TIMER
    fx.gd = collision_step( fx.gd, cell_states_, 3.0 );
    END_TIMER
})

PROFILE_AVG( tbb_thread_pool, collision_step, f, {

    set_tunnel_inlet( fx.gd, 0.01, 0.0 );
    streaming_step( fx.gd, cell_states_ );
    flush_tunnel_walls( fx.gd );

    START_TIMER
    fx.gd = collision_step_tbb_thread_pool( fx.gd, cell_states_, 3.0 );
    END_TIMER
})

PROFILE_AVG( dpcpp, collision_step, f, {

    set_tunnel_inlet( fx.gd, 0.01, 0.0 );
    streaming_step( fx.gd, cell_states_ );
    flush_tunnel_walls( fx.gd );

    START_TIMER
    fx.gd = collision_step_dpcpp( fx.gd, cell_states_, 3.0 );
    END_TIMER
})

PROFILE_AVG( dcpp_opt, collision_step, f, {

    set_tunnel_inlet( fx.gd, 0.01, 0.0 );
    streaming_step( fx.gd, cell_states_ );
    flush_tunnel_walls( fx.gd );

    START_TIMER
    fx.gd = collision_step_dpcpp_opt( fx.gd, cell_states_, 3.0 );
    END_TIMER
})

PROFILE_AVG( dcpp_opt_0, collision_step, f, {

    set_tunnel_inlet( fx.gd, 0.01, 0.0 );
    streaming_step( fx.gd, cell_states_ );
    flush_tunnel_walls( fx.gd );

    START_TIMER
    fx.gd = collision_step_dpcpp_opt_0( fx.gd, cell_states_, 3.0 );
    END_TIMER
})

// GENERATE_GRAPH( collision_step )


