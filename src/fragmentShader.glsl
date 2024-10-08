#version 330 core

uniform vec3 camPos; // Camera position
uniform vec3 lightPos; // Light position (sun's position)
uniform sampler2D texture1; // Texture sampler
uniform vec3 objectColor; // Color of the object (planet)
uniform bool useTexture; // Flag to determine whether to use texture
uniform bool isSun; // Flag to determine if the object is the sun
uniform bool isSky; // Flag to determine if the object is the skybox

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 color; // shader output: color of this fragment

void main() {
    if (isSky) {
        // Apply the sky texture directly
        FragColor = texture(texture1, TexCoord);
        return;
    }

    vec3 n = normalize(fNormal);
    vec3 l = normalize(lightPos - fPosition); // Light direction from light source to fragment
    vec3 v = normalize(camPos - fPosition); // View direction
    vec3 r = reflect(-l, n); // Reflected light direction

    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(texture1, fTexCoord).rgb;
    } else {
        baseColor = objectColor;
    }

    vec3 ambient = baseColor * vec3(0.5, 0.5, 0.5); // Ambient light

    vec3 lighting;
    if (isSun) {
        lighting = ambient; // Sun is lit by only its own ambient lighting

    } else {
        vec3 diffuse = baseColor * vec3(1.0, 1.0, 1.0) * max(dot(n, l), 0.0); // Diffuse light
        vec3 specular = vec3(0.5, 0.5, 0.5) * pow(max(dot(r, v), 0.0), 32); // Specular light
        lighting = ambient + diffuse + specular; // Combine lighting components
    }

    color = vec4(lighting, 1.0); // Final color (RGBA from RGB)
}