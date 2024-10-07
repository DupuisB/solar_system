#version 330 core

layout(location = 0) in vec3 aPos; // Position attribute
layout(location = 1) in vec3 aColor; // Color attribute
layout(location = 2) in vec3 vNormal; // Normal attribute

out vec3 vertexColor; // Output color to the fragment shader
out vec3 fNormal; // Output normal to the fragment shader

void main() {
    gl_Position = vec4(aPos, 1.0); // Set the position of the vertex
    vertexColor = aColor; // Pass the color to the fragment shader
    fNormal = vNormal; // Pass the normal to the fragment shader
}
