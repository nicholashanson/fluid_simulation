#version 330 core

layout( location = 0 ) in vec2 aPos;   // vertex position ( x, y )
layout( location = 1 ) in vec3 aColor; // vertex color ( r, g, b )

out vec3 vertexColor;                   // pass the color to the fragment shader

void main() {
    gl_Position = vec4( aPos, 0.0, 1.0 );  // convert 2D position to 4D homogeneous coordinates
    vertexColor = aColor;                   // pass the vertex color to the fragment shader
}
