#version 330 core

uniform vec3 camPos;         // Camera position
uniform vec3 lightPos;       // Light position (sun's position)
uniform sampler2D texture1;  // Texture sampler
uniform vec3 objectColor;    // Color of the object (planet)
uniform bool useTexture;     // Flag to determine whether to use texture
uniform bool isSun;          // Flag to determine if the object is the sun

in vec3 fPosition; // Fragment position in world space
in vec3 fNormal;   // Fragment normal in world space
in vec2 fTexCoord; // Fragment texture coordinate

out vec4 color; // shader output: color of this fragment

void main() {
    vec3 n = normalize(fNormal);
    vec3 l = normalize(lightPos - fPosition); // Light direction from light source to fragment
    vec3 v = normalize(camPos - fPosition);   // View direction
    vec3 r = reflect(-l, n);                  // Reflected light direction

    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(texture1, fTexCoord).rgb;
    } else {
        baseColor = objectColor;
    }

    vec3 lighting;
    if (isSun) {
        lighting = baseColor * vec3(0.8, 0.8, 0.8); // Sun is lit by only its own ambient lighting
    } else {
        vec3 ambient = baseColor * vec3(0.5, 0.5, 0.5); // Ambient light
        vec3 diffuse = baseColor * vec3(1.0, 1.0, 1.0) * max(dot(n, l), 0.0); // Diffuse light
        vec3 specular = vec3(0.5, 0.5, 0.5) * pow(max(dot(r, v), 0.0), 32); // Specular light
        lighting = ambient + diffuse + specular; // Combine lighting components
    }

    color = vec4(lighting, 1.0); // Final color (RGBA from RGB)
}
