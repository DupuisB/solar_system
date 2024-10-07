#version 330 core

uniform vec3 camPos; // Camera position
in vec3 fPosition; // Fragment position
in vec3 fNormal; // Fragment normal
out vec4 FragColor; // Output color

void main() {
	vec3 n = normalize(fNormal);
	vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // Light direction vector (hard-coded just for now)
	vec3 v = normalize(camPos - fPosition); // View vector
	vec3 r = reflect(-l, n); // Reflection vector

	// Ambient color
	vec3 ambient = vec3(0.1, 0.1, 0.1); // Ambient color (can be adjusted)

	// Diffuse lighting
	float diff = max(dot(n, l), 0.0);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0); // Diffuse color (can be adjusted)

	// Specular lighting
	float spec = pow(max(dot(v, r), 0.0), 32.0); // Shininess factor (32.0 can be adjusted)
	vec3 specular = spec * vec3(1.0, 1.0, 1.0); // Specular color (can be adjusted)

	// Combine all components
	vec3 color = ambient + diffuse + specular;

	FragColor = vec4(color, 1.0); // Set the fragment color
}