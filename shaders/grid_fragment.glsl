#version 330 core

in vec3 vertexColor; // interpolated color from the vertex shader
out vec4 FragColor;  // final fragment color

void main() {
    FragColor = vec4( vertexColor, 1.0 ); // set the output color with full opacityw
}
