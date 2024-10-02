#version 330 core	     // Minimal GL version support expected from the GPU

out vec4 color;	  // Shader output: the color response attached to this fragment

void main() {
	color = vec4(1.0); // build an RGBA from an RGB
}
