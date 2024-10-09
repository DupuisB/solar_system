// Processes the vertex data: from local space to clip space. Next stage is the rasterizer, then the fragment shader.
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

out vec3 fPosition; // Fragment position in world space
out vec3 fNormal;   // Fragment normal in world space
out vec2 fTexCoord; // Fragment texture coordinate

void main()
{
    // Transform the vertex position to world space
    vec4 worldPosition = modelMat * vec4(aPos, 1.0);
    fPosition = worldPosition.xyz;

    // Transform the normal to world space
    fNormal = mat3(transpose(inverse(modelMat))) * aNormal;

    // Pass the texture coordinate to the fragment shader
    fTexCoord = aTexCoord;

    // Transform the vertex position to clip space
    gl_Position = projMat * viewMat * worldPosition;
}