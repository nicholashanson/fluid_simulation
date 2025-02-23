#ifndef GL_HPP
#define GL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include <settings.hpp>
#include <fs/fs.hpp>

GLFWwindow* initialize_window();

unsigned int setup_openGL();

void projection( int shader_program );

#endif

