#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glad/glad.h>

namespace app {

    unsigned int setup_shaders( const std::string& vertex_path, const std::string& fragment_path );

}

#endif
