#include <shader.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace fs {

    static std::string read_shader_source( const std::string& file_path ) {

        std::ifstream file( file_path );

        if ( !file.is_open() ) {
            throw std::runtime_error( "failed to open shader file: " + file_path );
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    static unsigned int compile_shader( const std::string& source, GLenum type ) {

        unsigned int shader = glCreateShader( type );

        const char * src = source.c_str();

        glShaderSource( shader, 1, &src, nullptr );

        glCompileShader( shader );

        // check for compilation errors
        int success;
        char info_log[ 1024 ];

        glGetShaderiv( shader, GL_COMPILE_STATUS, &success );

        if ( !success ) {

            glGetShaderInfoLog( shader, 1024, nullptr, info_log );

            std::cerr << "ERROR::SHADER::COMPILATION_ERROR\n" << info_log << std::endl;

            throw std::runtime_error( "shader compilation failed" );
        }

        return shader;

    }

    unsigned int setup_shaders( const std::string& vertex_path, const std::string& fragment_path ) {

        std::string vertex_code = read_shader_source( vertex_path );
        std::string fragment_code = read_shader_source( fragment_path );

        unsigned int vertex_shader = compile_shader( vertex_code, GL_VERTEX_SHADER );
        unsigned int fragment_shader = compile_shader( fragment_code, GL_FRAGMENT_SHADER );

        unsigned int program = glCreateProgram();
        glAttachShader( program, vertex_shader );
        glAttachShader( program, fragment_shader );
        glLinkProgram( program );

        // check for linking errors
        int success;
        char info_log[ 1024 ];

        glGetProgramiv( program, GL_LINK_STATUS, &success );

        if ( !success ) {

            glGetProgramInfoLog( program, 1024, nullptr, info_log );

            std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;

            throw std::runtime_error( "program linking failed ");
        }

        glDeleteShader( vertex_shader );
        glDeleteShader( fragment_shader );

        return program;
    }

}




