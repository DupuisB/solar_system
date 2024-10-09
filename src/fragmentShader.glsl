// Fragment shader: This shader is responsible for calculating the color of each fragment (pixel) of the object.

#version 330 core

uniform vec3 camPos; // Camera position
uniform vec3 lightPos; // Light position (sun's position)
uniform sampler2D texture1; // Texture sampler
uniform vec3 objectColor; // Color of the object (planet)
uniform bool useTexture; // Flag to determine whether to use texture

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 color; // shader output: color of this fragment

void main() {
    vec3 n = normalize(fNormal);
    vec3 l = normalize(vec3(-1.0, -1.0, 0.0)); // Light direction from light source to fragment
    vec3 v = normalize(camPos - fPosition); // View direction
    vec3 r = reflect(-l, n); // Reflected light direction

    // vec3 baseColor = useTexture ? texture(texture1, fTexCoord).rgb : objectColor;
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(texture1, fTexCoord).rgb;
    } else {
        baseColor = objectColor;
    }

    vec3 ambient = baseColor * vec3(0.1, 0.1, 0.1); // Ambient light
    vec3 diffuse = baseColor * vec3(0.5, 0.5, 0.5) * max(dot(n, l), 0.0); // Diffuse light
    vec3 specular = vec3(0.5, 0.5, 0.5) * pow(max(dot(r, v), 0.0), 32); // Specular light

    vec3 lighting = ambient + diffuse + specular; // Combine lighting components
    color = vec4(lighting, 1.0); // Final color (RGBA from RGB)
}