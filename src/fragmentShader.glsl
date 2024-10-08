#version 330 core

uniform vec3 camPos; // Camera position
in vec3 fPosition;
in vec3 fNormal;

out vec4 color; // shader output: color of this fragment

void main() {
        vec3 n = normalize(fNormal);
        vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // Light direction (hardcoded)
        vec3 v = normalize(camPos - fPosition); // View direction
        vec3 r = reflect(-l, n); // Reflected light direction

        vec3 ambient = vec3(0.1, 0.1, 0.1); // Ambient light
        vec3 diffuse = vec3(0.5, 0.5, 0.5) * max(dot(n, l), 0.0); // Diffuse light
        vec3 specular = vec3(0.5, 0.5, 0.5) * pow(max(dot(r, v), 0.0), 32); // Specular light

        color = vec4(ambient + diffuse + specular, 1.0); // Final color (RGBA from RGB)
}