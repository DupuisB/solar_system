layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vColor;
uniform mat4 viewMat, projMat;
out vec3 fColor;

void main() {
gl_Position = projMat * viewMat * vec4(vPosition, 1.0); // mandatory to rasterize properly
fColor = vColor; // will be passed to the next stage
}