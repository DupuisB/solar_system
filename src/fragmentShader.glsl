#version 330 core

uniform vec3 camPos; // Camera position
uniform vec3 lightPos; // Light position (sun's position)
uniform sampler2D texture1; // Texture sampler
uniform vec3 objectColor; // Color of the object (planet)
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 color; // shader output: color of this fragment

void main() {
    vec3 n = normalize(fNormal);
    vec3 l = normalize(lightPos - fPosition); // Light direction from light source to fragment
    vec3 v = normalize(camPos - fPosition); // View direction
    vec3 r = reflect(-l, n); // Reflected light direction

    vec3 ambient = objectColor * vec3(0.1, 0.1, 0.1); // Ambient light
    vec3 diffuse = objectColor * vec3(0.5, 0.5, 0.5) * max(dot(n, l), 0.0); // Diffuse light
    vec3 specular = vec3(0.5, 0.5, 0.5) * pow(max(dot(r, v), 0.0), 32); // Specular light

    vec4 texColor = texture(texture1, fTexCoord); // Sample the texture
    color = vec4((ambient + diffuse + specular) * texColor.rgb, texColor.a); // Final color (RGBA from RGB)
}