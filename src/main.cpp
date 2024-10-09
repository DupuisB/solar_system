// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2024 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// [Project 1] IGR201 Practical: OpenGL and Shaders
//
// This assignment aims to create a simple 3D scene with a sun, an earth, and a moon.
// The sun is at the center of the scene, the earth orbits around the sun, and the moon orbits around the earth.
// The sun acts as a light source, and the earth and the moon are textured with images of the earth and the moon.
// The scene is viewed from a camera that can be moved around the scene.
//
// Libraries: OpenGL, GLFW, GLAD, GLM, STB
// What is a VBO: stores vertices in the GPU memory
// What is a VAO: stores the state of the VBOs (position, color, etc.)
//------------------------------------------------------------------------------


// https://assetstore.unity.com/packages/3d/environments/sci-fi/real-stars-skybox-lite-116333
#define _USE_MATH_DEFINES

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

const static float earthRotationPeriod = 1;
const static float earthOrbitPeriod = earthRotationPeriod * 2;
const static float earthTilt = 23.5;
const static float moonRotationPeriod = earthRotationPeriod / 2;
const static float moonOrbitPeriod = moonRotationPeriod;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_colVbo = 0;
GLuint g_ibo = 0;

glm::mat4 g_sun = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeSun));
glm::mat4 g_earth = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));
glm::mat4 g_moon = glm::translate(g_earth, glm::vec3(kRadOrbitMoon, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));

glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellowish

// Basic camera model
class Camera
{
public:
    inline float getFov() const { return m_fov; }
    inline void setFoV(const float f) { m_fov = f; }
    inline float getAspectRatio() const { return m_aspectRatio; }
    inline void setAspectRatio(const float a) { m_aspectRatio = a; }
    inline float getNear() const { return m_near; }
    inline void setNear(const float n) { m_near = n; }
    inline float getFar() const { return m_far; }
    inline void setFar(const float n) { m_far = n; }
    inline void setPosition(const glm::vec3 &p) { m_pos = p; }
    inline glm::vec3 getPosition() { return m_pos; }

    inline glm::mat4 computeViewMatrix() const
    {
        return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }

    // Returns the projection matrix stemming from the camera intrinsic parameter.
    inline glm::mat4 computeProjectionMatrix() const
    {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
    }

private:
    glm::vec3 m_pos = glm::vec3(0, 0, 0);
    float m_fov = 45.f;        // Field of view, in degrees
    float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
    float m_near = 0.1f;       // Distance before which geometry is excluded from the rasterization process
    float m_far = 10.f;        // Distance after which the geometry is excluded from the rasterization process
};

Camera g_camera;

GLuint loadTextureFromFileToGPU(const std::string &filename) {
    // Loading the image in CPU memory using stb_image
    int width, height, numComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);

    if (data == nullptr) {
        std::cerr << "Error: texture " << filename << " not found" << std::endl;
        return 0;
    }

    std::cout << "Loaded texture " << filename << " with width=" << width << " height=" << height << " numComponents=" << numComponents << std::endl;

    GLuint texID; // OpenGL texture identifier
    glGenTextures(1, &texID); // generate a texture object
    glBindTexture(GL_TEXTURE_2D, texID); // bind the texture object to the 2D texture target (now active texture)

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear interpolation for magnifying the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear interpolation for minifying the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat the texture outside the [0, 1] range (horizontal)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat the texture outside the [0, 1] range (vertical)


    // Fill the GPU texture with the data stored in the CPU image
    // Parameters: texture target, mipmap level, internal format, width, height, border, format, type, data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    // Free useless CPU memory
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture (now the active texture is 0)
    return texID; // return the texture id (useful to use it later)
}

class Mesh {
public:
    void init() {
        glGenVertexArrays(1, &m_vao); // Generate 1 vertex array object (VAO)
        glBindVertexArray(m_vao); // Bind the VAO to the current context (now the active VAO)

        glGenBuffers(1, &m_posVbo); // Generate 1 vertex buffer object (VBO)
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo); // Bind the VBO to the current context (now the active VBO)
        glBufferData(GL_ARRAY_BUFFER, m_vertexPositions.size() * sizeof(float), m_vertexPositions.data(), GL_STATIC_DRAW); // Fill the VBO with the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // Define the layout of the vertex data in the VBO (attrib index, number of components, type, normalized, stride, pointer)
        glEnableVertexAttribArray(0); // Enable the attribute at index 0, now accessible in the vertex shader (layout(location = 0))

        glGenBuffers(1, &m_normalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexNormals.size() * sizeof(float), m_vertexNormals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &m_texCoordVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexTexCoords.size() * sizeof(float), m_vertexTexCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(2);

        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangleIndices.size() * sizeof(unsigned int), m_triangleIndices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0); // Unbind the VAO
    }

    void render() {
        glBindVertexArray(m_vao); // Bind the VAO to the current context (now the active VAO)
        glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles using the indices in the IBO (mode, count, type, indices)
        glBindVertexArray(0); // Unbind the VAO
    }

    // Implementation of genSphere
    std::shared_ptr<Mesh> genSphere(const size_t resolution = 16) {
        auto mesh = std::make_shared<Mesh>();
        float dTheta = 2.0f * M_PI / resolution;
        float dPhi = M_PI / resolution;

        // Generate vertices
        for (size_t i = 0; i <= resolution; ++i) {
            float phi = i * dPhi;
            for (size_t j = 0; j <= resolution; ++j) {
                float theta = j * dTheta;
                
                // Convert spherical coordinates to Cartesian coordinates
                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);

                // Add vertex position
                mesh->m_vertexPositions.push_back(x);
                mesh->m_vertexPositions.push_back(y);
                mesh->m_vertexPositions.push_back(z);

                // Add vertex normal (same as position for unit sphere)
                mesh->m_vertexNormals.push_back(x);
                mesh->m_vertexNormals.push_back(y);
                mesh->m_vertexNormals.push_back(z);

                // Add texture coordinates
                mesh->m_vertexTexCoords.push_back(static_cast<float>(j) / resolution);
                mesh->m_vertexTexCoords.push_back(static_cast<float>(i) / resolution);

                // Add triangle indices
                if (i < resolution && j < resolution) {
                    size_t idx = i * (resolution + 1) + j;
                    mesh->m_triangleIndices.push_back(idx);
                    mesh->m_triangleIndices.push_back(idx + resolution + 1);
                    mesh->m_triangleIndices.push_back(idx + resolution + 2);

                    mesh->m_triangleIndices.push_back(idx);
                    mesh->m_triangleIndices.push_back(idx + resolution + 2);
                    mesh->m_triangleIndices.push_back(idx + 1);
                }
            }
        }
        return mesh;
    }

private:
    std::vector<float> m_vertexPositions; // (VBO)
    std::vector<float> m_vertexNormals; // (VBO)
    std::vector<unsigned int> m_triangleIndices; // (IBO)
    std::vector<float> m_vertexTexCoords; // (VBO)
    GLuint m_texCoordVbo = 0;
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;
};

std::shared_ptr<Mesh> g_sphereMesh = std::make_shared<Mesh>(); // Sphere mesh

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height)
{
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_W)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (action == GLFW_PRESS && key == GLFW_KEY_F)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_A))
    {
        glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
    }
}

void errorCallback(int error, const char *desc)
{
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW()
{
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit())
    {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Request OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Request OpenGL version 3.3
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Avoid using deprecated functionality
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile (more modern functions)
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // The window is resizable

    // Create the window
    g_window = glfwCreateWindow(
        1024, 1024,
        "Interactive 3D Applications (OpenGL) - Simple Solar System",
        nullptr, nullptr);
    if (!g_window)
    {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window); // Make the created window's context current
    glfwSetWindowSizeCallback(g_window, windowSizeCallback); // Set the callback for window resizing 
    glfwSetKeyCallback(g_window, keyCallback); // Set the callback for key entering
}

void initOpenGL()
{
    // Load extensions for modern OpenGL
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK);                  // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE);               // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);                 // Specify the depth test for the z-buffer
    glEnable(GL_DEPTH_TEST);              // Enable the z-buffer test in the rasterization
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename)
{
    std::ifstream t(filename.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename)
{
    GLuint shader = glCreateShader(type);                                    // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
    std::string shaderSourceString = file2String(shaderFilename);            // Loads the shader source from a file to a C++ string
    const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
    glShaderSource(shader, 1, &shaderSource, NULL);                          // load the vertex shader code
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader); // Attach the shader to the program
    glDeleteShader(shader); // The shader is now in the program, it can be deleted
}

void initGPUprogram()
{
    g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl"); // Load the vertex shader
    loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl"); // Load the fragment shader
    glLinkProgram(g_program); // The main GPU program is ready to handle streams of polygons

    glUseProgram(g_program); // Activate the GPU program
    // TODO: set shader variables, textures, etc.


}

// Define your mesh(es) in the CPU memory
void initCPUgeometry()
{
    g_sphereMesh = g_sphereMesh->genSphere(32);
}

void initGPUgeometry()
{
    g_sphereMesh->init();
}

void initCamera()
{
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    // Set the camera position to capture all three planets
    g_camera.setPosition(glm::vec3(0.0, 20.0, 30.0)); // Adjust as needed

    // Set near and far planes
    g_camera.setNear(0.1);
    g_camera.setFar(80.1);
}

GLuint earthTexture, moonTexture;

void initTextures() {
    earthTexture = loadTextureFromFileToGPU("./media/earth.jpg");
    moonTexture = loadTextureFromFileToGPU("./media/moon.jpg");
}

void init()
{
    initGLFW();
    initOpenGL();
    initCPUgeometry();
    initGPUprogram();
    initGPUgeometry();
    initCamera();
    initTextures();

    // Initialize time
    glfwSetTime(0.0);
}

void clear()
{
    glDeleteProgram(g_program);

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

    // Params: program, location, count, transpose, value
    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix));

    const glm::vec3 camPosition = g_camera.getPosition();
    glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

    // Set the light position (sun's position)
    glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Sun is at the origin
    glUniform3fv(glGetUniformLocation(g_program, "lightPos"), 1, glm::value_ptr(lightPosition));

    // Render the sun (no texture)
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_sun));
    glUniform3fv(glGetUniformLocation(g_program, "objectColor"), 1, glm::value_ptr(sunColor));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_FALSE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_TRUE); // Set isSun to true
    g_sphereMesh->render();

    // Render the earth
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_earth));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_FALSE); // Set isSun to false
    g_sphereMesh->render();

    // Render the moon
    glBindTexture(GL_TEXTURE_2D, moonTexture);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_moon));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_FALSE); // Set isSun to false
    g_sphereMesh->render();
}
// Update any accessible variable based on the current time
void update(const float currentTimeInSec)
{
    // Get the current time in seconds
    float currentTimeGLFW = static_cast<float>(glfwGetTime());

    // Update the earth position
    float earthAngle = currentTimeGLFW / earthOrbitPeriod * 2 * M_PI;
    g_earth = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth * cos(earthAngle), 0.0f, kRadOrbitEarth * sin(earthAngle))) * glm::rotate(glm::mat4(1.0f), earthAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));

    // Update the moon position
    float moonAngle = currentTimeInSec / moonRotationPeriod * 2 * M_PI;
    g_moon = glm::translate(g_earth, glm::vec3(kRadOrbitMoon * cos(moonAngle), 0.0f, kRadOrbitMoon * sin(moonAngle))) * glm::rotate(glm::mat4(1.0f), moonAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));
}

int main(int argc, char **argv)
{
    init();                                         // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
    while (!glfwWindowShouldClose(g_window))        // The rendering loop
    {
        update(static_cast<float>(glfwGetTime()));  // Update function
        render();                                   // Rendering function
        glfwSwapBuffers(g_window);                  // Swap the front and back buffers (avoid flickering)
        glfwPollEvents();                           // Process the events (keyboard, mouse, etc)
    }
    clear();
    return EXIT_SUCCESS;
}