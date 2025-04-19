#ifndef TEST_CONSTANTS_HPP
#define TEST_CONSTANTS_HPP

#include <vector>

namespace test {

    inline const std::vector<std::pair<double, double>> square_points = { {  0.0,  0.0 }, 
                                                                          { 10.0,  0.0 }, 
                                                                          { 10.0, 10.0 }, 
                                                                          {  0.0, 10.0 } };

    inline const std::vector<std::vector<size_t>> square_boundary_nodes = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };
}

#endif