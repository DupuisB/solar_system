// main.cpp
// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (Modified to include Saturn's rings)
//
// ----------------------------------------------------------------------------

//------------------------------------------------------------------------------
/*
[Project 1] IGR201 Practical: OpenGL and Shaders

This assignment creates a simple 3D solar system with the Sun, Earth, Moon, and Saturn.
- The Sun is at the center and acts as a light source.
- The Earth orbits the Sun and rotates around its own axis.
- The Moon orbits the Earth and rotates around its own axis.
- Saturn orbits the Sun and rotates around its own axis.
- Saturn has rings textured with a provided colormap (rings are not working properly)
- Textures are applied to Earth, Moon, and Saturn for realism.
- A skybox provides a background environment.
- The camera can be moved and zoomed to observe the scene from different angles.
- The simulation can be paused and resumed with the 'F' key.

Libraries Used:
- OpenGL
- GLFW
- GLAD
- GLM
- STB Image

What is a VBO: Stores vertices in the GPU memory.
What is a VAO: Stores the state of VBOs (position, normals, texture coordinates, etc.).
*/
//------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <cstdlib>

// Include OpenGL headers
#include <glad/gl.h>
#include <GLFW/glfw3.h>

// Include GLM for transformations
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Include STB for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Define PI constant
const float PI = 3.14159265358979323846f;

// Constants for celestial bodies
const static float kSizeSun = 1.0f;

const static float kSizeEarth = 0.5f;
const static float kRadOrbitEarth = 10.0f;
const static float earthOrbitPeriod = 4.0f;
const static float earthRotationPeriod = earthOrbitPeriod / 2.0f;

const static float kSizeMoon = 0.25f;
const static float kRadOrbitMoon = 2.0f;
const static float moonOrbitPeriod = 2.0f; 
const static float moonRotationPeriod = moonOrbitPeriod; 

const static float kSizeSaturn = 0.75f;
const static float kRadOrbitSaturn = 25.0f;
const static float saturnOrbitPeriod = 20.0f;
const static float saturnRotationPeriod = saturnOrbitPeriod / 3.0f;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU programs
GLuint g_program = 0;         // Main shader program
GLuint skyboxProgram = 0;     // Skybox shader program

// Matrices for celestial bodies
glm::mat4 g_sun = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeSun));
glm::mat4 g_earth = glm::mat4(1.0f);
glm::mat4 g_moon = glm::mat4(1.0f);
glm::mat4 g_saturn = glm::mat4(1.0f);

// Sun color
glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellowish

// Camera class with movement and zoom support
class Camera {
public:
    Camera()
        : m_pos(0.0f, 0.0f, 30.0f),
          m_front(0.0f, 0.0f, -1.0f),
          m_up(0.0f, 1.0f, 0.0f),
          m_yaw(-90.0f),
          m_pitch(0.0f),
          m_speed(5.0f),
          m_sensitivity(0.1f),
          m_firstMouse(true),
          m_lastX(0.0f),
          m_lastY(0.0f),
          m_fov(45.0f),
          m_aspectRatio(1.0f),
          m_near(0.1f),
          m_far(100.0f) {}

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

    inline glm::mat4 computeViewMatrix() const {
        return glm::lookAt(m_pos, m_pos + m_front, m_up);
    }

    inline glm::mat4 computeProjectionMatrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
    }

    void processKeyboard(int key, float deltaTime) {
        float velocity = m_speed * deltaTime;
        if (key == GLFW_KEY_W)
            m_pos += m_front * velocity;
        if (key == GLFW_KEY_S)
            m_pos -= m_front * velocity;
        if (key == GLFW_KEY_A)
            m_pos -= glm::normalize(glm::cross(m_front, m_up)) * velocity;
        if (key == GLFW_KEY_D)
            m_pos += glm::normalize(glm::cross(m_front, m_up)) * velocity;
        if (key == GLFW_KEY_SPACE)
            m_pos += m_up * velocity;
        if (key == GLFW_KEY_LEFT_SHIFT)
            m_pos -= m_up * velocity;
    }

    void processKeyboardZoom(bool zoomIn, float deltaTime) {
        float zoomSpeed = 20.0f * deltaTime;
        if (zoomIn) {
            m_fov -= zoomSpeed;
            if (m_fov < 1.0f)
                m_fov = 1.0f;
        } else {
            m_fov += zoomSpeed;
            if (m_fov > 45.0f)
                m_fov = 45.0f;
        }
    }

    void processMouseMovement(float xpos, float ypos) {
        if (m_firstMouse) {
            m_lastX = xpos;
            m_lastY = ypos;
            m_firstMouse = false;
        }

        float xoffset = xpos - m_lastX;
        float yoffset = m_lastY - ypos;
        m_lastX = xpos;
        m_lastY = ypos;

        xoffset *= m_sensitivity;
        yoffset *= m_sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;

        updateCameraVectors();
    }

    void processMouseScroll(float yoffset) {
        m_fov -= yoffset;
        if (m_fov < 1.0f)
            m_fov = 1.0f;
        if (m_fov > 45.0f)
            m_fov = 45.0f;
    }

    void updateCameraVectors() {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
        
        // also re-calculate the Right and Up vector
        m_right = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_up    = glm::normalize(glm::cross(m_right, m_front));
    }

private:
    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    float m_yaw;
    float m_pitch;
    float m_speed;
    float m_sensitivity;
    float m_fov;
    float m_aspectRatio;
    float m_near;
    float m_far;
    bool m_firstMouse;
    float m_lastX;
    float m_lastY;
};

Camera g_camera;

// Function to load a texture from file and upload to GPU
GLuint loadTextureFromFileToGPU(const std::string &filename) {
    int width, height, numComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);

    if (data == nullptr) {
        std::cerr << "Error: texture " << filename << " not found" << std::endl;
        return 0;
    }

    std::cout << "Loaded texture " << filename << " with width=" << width
              << " height=" << height << " numComponents=" << numComponents << std::endl;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format = GL_RGB;
    if (numComponents == 1)
        format = GL_RED;
    else if (numComponents == 3)
        format = GL_RGB;
    else if (numComponents == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

// Function to load cubemap textures
GLuint loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, numComponents;
    for (GLuint i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &numComponents, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (numComponents == 1)
                format = GL_RED;
            else if (numComponents == 3)
                format = GL_RGB;
            else if (numComponents == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
            return 0;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    return textureID;
}

class Mesh {
public:
    void init() {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexPositions.size() * sizeof(float), m_vertexPositions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        if (!m_vertexNormals.empty()) {
            glGenBuffers(1, &m_normalVbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
            glBufferData(GL_ARRAY_BUFFER, m_vertexNormals.size() * sizeof(float), m_vertexNormals.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(1);
        }

        if (!m_vertexTexCoords.empty()) {
            glGenBuffers(1, &m_texCoordVbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
            glBufferData(GL_ARRAY_BUFFER, m_vertexTexCoords.size() * sizeof(float), m_vertexTexCoords.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(2);
        }

        if (!m_triangleIndices.empty()) {
            glGenBuffers(1, &m_ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangleIndices.size() * sizeof(unsigned int), m_triangleIndices.data(), GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
    }

    void render() {
        glBindVertexArray(m_vao);
        if (!m_triangleIndices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_triangleIndices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertexPositions.size() / 3));
        }
        glBindVertexArray(0);
    }

    // Generate a sphere mesh with updated math to match user's code
    static std::shared_ptr<Mesh> genSphere(const size_t resolution=16){
        auto mesh = std::make_shared<Mesh>();
        for (size_t lat = 0; lat <= resolution; ++lat) {
            float phi = lat * PI / resolution;
            for (size_t lon = 0; lon <= resolution; ++lon) {
                float theta = lon * 2.0f * PI / resolution;

                // Updated math: x = sin(phi) * sin(theta), z = sin(phi) * cos(theta)
                float x = sin(phi) * sin(theta);
                float y = cos(phi);
                float z = sin(phi) * cos(theta);

                mesh->m_vertexPositions.push_back(x);
                mesh->m_vertexPositions.push_back(y);
                mesh->m_vertexPositions.push_back(z);

                mesh->m_vertexNormals.push_back(x);
                mesh->m_vertexNormals.push_back(y);
                mesh->m_vertexNormals.push_back(z);

                float u = static_cast<float>(lon) / resolution;
                float v = static_cast<float>(lat) / resolution;

                mesh->m_vertexTexCoords.push_back(u);
                mesh->m_vertexTexCoords.push_back(v);

                if (lat < resolution && lon < resolution) {
                    size_t idx = lat * (resolution + 1) + lon;
                    mesh->m_triangleIndices.push_back(static_cast<unsigned int>(idx));
                    mesh->m_triangleIndices.push_back(static_cast<unsigned int>(idx + resolution + 1));
                    mesh->m_triangleIndices.push_back(static_cast<unsigned int>(idx + resolution + 2));

                    mesh->m_triangleIndices.push_back(static_cast<unsigned int>(idx));
                    mesh->m_triangleIndices.push_back(static_cast<unsigned int>(idx + resolution + 2));
                    mesh->m_triangleIndices.push_back(static_cast<unsigned int>(idx + 1));
                }
            }
        }
        return mesh;
    }

    // Generate a cube mesh for skybox
    static std::shared_ptr<Mesh> genCube() {
        auto mesh = std::make_shared<Mesh>();
        float vertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,  
            -1.0f, -1.0f, -1.0f,  
             1.0f, -1.0f, -1.0f,  
             1.0f, -1.0f, -1.0f,  
             1.0f,  1.0f, -1.0f,  
            -1.0f,  1.0f, -1.0f,  

            -1.0f, -1.0f,  1.0f,  
            -1.0f, -1.0f, -1.0f,  
            -1.0f,  1.0f, -1.0f,  
            -1.0f,  1.0f, -1.0f,  
            -1.0f,  1.0f,  1.0f,  
            -1.0f, -1.0f,  1.0f,  

             1.0f, -1.0f, -1.0f,  
             1.0f, -1.0f,  1.0f,  
             1.0f,  1.0f,  1.0f,  
             1.0f,  1.0f,  1.0f,  
             1.0f,  1.0f, -1.0f,  
             1.0f, -1.0f, -1.0f,  

            -1.0f, -1.0f,  1.0f,  
            -1.0f,  1.0f,  1.0f,  
             1.0f,  1.0f,  1.0f,  
             1.0f,  1.0f,  1.0f,  
             1.0f, -1.0f,  1.0f,  
            -1.0f, -1.0f,  1.0f,  

            -1.0f,  1.0f, -1.0f,  
             1.0f,  1.0f, -1.0f,  
             1.0f,  1.0f,  1.0f,  
             1.0f,  1.0f,  1.0f,  
            -1.0f,  1.0f,  1.0f,  
            -1.0f,  1.0f, -1.0f,  

            -1.0f, -1.0f, -1.0f,  
            -1.0f, -1.0f,  1.0f,  
             1.0f, -1.0f, -1.0f,  
             1.0f, -1.0f, -1.0f,  
            -1.0f, -1.0f,  1.0f,  
             1.0f, -1.0f,  1.0f   
        };
        mesh->m_vertexPositions.assign(vertices, vertices + sizeof(vertices) / sizeof(float));
        return mesh;
    }

    // Generate a ring mesh (annulus) with given inner and outer radii and resolution
    static std::shared_ptr<Mesh> genRing(float innerRadius, float outerRadius, size_t resolution = 64) {
        auto mesh = std::make_shared<Mesh>();
        for (size_t i = 0; i <= resolution; ++i) {
            float theta = i * 2.0f * PI / resolution;
            float cosTheta = cos(theta);
            float sinTheta = sin(theta);

            // Outer vertex
            float x_outer = outerRadius * cosTheta;
            float z_outer = outerRadius * sinTheta;
            mesh->m_vertexPositions.push_back(x_outer);
            mesh->m_vertexPositions.push_back(0.0f); // Flat ring in XZ plane
            mesh->m_vertexPositions.push_back(z_outer);

            // Inner vertex
            float x_inner = innerRadius * cosTheta;
            float z_inner = innerRadius * sinTheta;
            mesh->m_vertexPositions.push_back(x_inner);
            mesh->m_vertexPositions.push_back(0.0f);
            mesh->m_vertexPositions.push_back(z_inner);

            // Normals (pointing up)
            mesh->m_vertexNormals.push_back(0.0f);
            mesh->m_vertexNormals.push_back(1.0f);
            mesh->m_vertexNormals.push_back(0.0f);

            mesh->m_vertexNormals.push_back(0.0f);
            mesh->m_vertexNormals.push_back(1.0f);
            mesh->m_vertexNormals.push_back(0.0f);

            // Texture coordinates
            float u = static_cast<float>(i) / resolution;
            mesh->m_vertexTexCoords.push_back(u);
            mesh->m_vertexTexCoords.push_back(0.0f); // Outer edge
            mesh->m_vertexTexCoords.push_back(u);
            mesh->m_vertexTexCoords.push_back(1.0f); // Inner edge
        }

        // Generate triangle strip indices
        for (size_t i = 0; i < resolution; ++i) {
            mesh->m_triangleIndices.push_back(static_cast<unsigned int>(2 * i));
            mesh->m_triangleIndices.push_back(static_cast<unsigned int>(2 * i + 1));
            mesh->m_triangleIndices.push_back(static_cast<unsigned int>(2 * (i + 1)));

            mesh->m_triangleIndices.push_back(static_cast<unsigned int>(2 * (i + 1)));
            mesh->m_triangleIndices.push_back(static_cast<unsigned int>(2 * i + 1));
            mesh->m_triangleIndices.push_back(static_cast<unsigned int>(2 * (i + 1) + 1));
        }

        return mesh;
    }

    // Generate a ring mesh with multiple layers (optional)
    // You can extend this method to support multiple ring layers if desired

    GLuint getVao() const { return m_vao; }

private:
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<float> m_vertexTexCoords;
    GLuint m_texCoordVbo = 0;
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;
};

// Declare the sphere, skybox, and ring meshes
std::shared_ptr<Mesh> g_sphereMesh = std::make_shared<Mesh>();
std::shared_ptr<Mesh> skyboxMesh = std::make_shared<Mesh>();
std::shared_ptr<Mesh> g_ringMesh = nullptr; // Added for Saturn's rings

// Skybox variables
GLuint cubemapTexture;

// Ring texture
GLuint ringTexture = 0;

// Time management
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// Simulation time and state
float simulationTime = 0.0f; // Custom simulation time
bool isSimulationFrozen = false; // Simulation frozen state

// Input handling
bool keys[1024] = { false };

// GLFW callbacks
void windowSizeCallback(GLFWwindow *window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);

    if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        isSimulationFrozen = !isSimulationFrozen;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    g_camera.processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    g_camera.processMouseScroll(static_cast<float>(yoffset));
}

void doMovement() {
    if (keys[GLFW_KEY_W])
        g_camera.processKeyboard(GLFW_KEY_W, deltaTime);
    if (keys[GLFW_KEY_S])
        g_camera.processKeyboard(GLFW_KEY_S, deltaTime);
    if (keys[GLFW_KEY_A])
        g_camera.processKeyboard(GLFW_KEY_A, deltaTime);
    if (keys[GLFW_KEY_D])
        g_camera.processKeyboard(GLFW_KEY_D, deltaTime);
    if (keys[GLFW_KEY_SPACE])
        g_camera.processKeyboard(GLFW_KEY_SPACE, deltaTime);
    if (keys[GLFW_KEY_LEFT_SHIFT])
        g_camera.processKeyboard(GLFW_KEY_LEFT_SHIFT, deltaTime);

    // Zoom key handling
    if (keys[GLFW_KEY_Q])
        g_camera.processKeyboardZoom(true, deltaTime); // Zoom in
    if (keys[GLFW_KEY_E])
        g_camera.processKeyboardZoom(false, deltaTime); // Zoom out
}

void errorCallback(int error, const char *desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    g_window = glfwCreateWindow(1024, 1024, "Simple Solar System with Saturn's Rings", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
    glfwSetCursorPosCallback(g_window, mouseCallback);
    glfwSetScrollCallback(g_window, scrollCallback);
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGL() {
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK);                  
    glEnable(GL_CULL_FACE);               
    glDepthFunc(GL_LESS);                 
    glEnable(GL_DEPTH_TEST);              
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f); 
}

// Load shader from file
std::string file2String(const std::string &filename) {
    std::ifstream t(filename.c_str());
    if (!t.is_open()) {
        std::cerr << "ERROR: Could not open shader file " << filename << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Load and compile shader
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
    GLuint shader = glCreateShader(type);                                    
    std::string shaderSourceString = file2String(shaderFilename);            
    const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); 
    glShaderSource(shader, 1, &shaderSource, NULL);                          
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader); 
    glDeleteShader(shader);
}

void initGPUprogram() {
    g_program = glCreateProgram();
    loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
    glLinkProgram(g_program);

    // Check for linking errors
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(g_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(g_program, 512, NULL, infoLog);
        std::cout << "ERROR: Shader program linking failed:\n" << infoLog << std::endl;
    }

    skyboxProgram = glCreateProgram();
    loadShader(skyboxProgram, GL_VERTEX_SHADER, "skyboxVertexShader.glsl");
    loadShader(skyboxProgram, GL_FRAGMENT_SHADER, "skyboxFragmentShader.glsl");
    glLinkProgram(skyboxProgram);

    // Check for linking errors
    glGetProgramiv(skyboxProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(skyboxProgram, 512, NULL, infoLog);
        std::cout << "ERROR: Skybox shader program linking failed:\n" << infoLog << std::endl;
    }

    // Set texture samplers
    glUseProgram(skyboxProgram);
    glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);

    glUseProgram(g_program);
    glUniform1i(glGetUniformLocation(g_program, "texture1"), 0);
}

void initCPUgeometry() {
    g_sphereMesh = Mesh::genSphere(16);
    skyboxMesh = Mesh::genCube();

    // Generate the ring mesh
    float innerRadius = kSizeSaturn * 1.5f;
    float outerRadius = kSizeSaturn * 2.5f;
    g_ringMesh = Mesh::genRing(innerRadius, outerRadius, 128);
}

void initGPUgeometry() {
    g_sphereMesh->init();
    skyboxMesh->init();

    // Initialize the ring mesh
    if (g_ringMesh) {
        g_ringMesh->init();
    }
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(0.0f, 0.0f, 30.0f));

    g_camera.setNear(0.1f);
    g_camera.setFar(100.0f);
}

GLuint earthTexture, moonTexture, saturnTexture, sunTexture;

void initTextures() {
    earthTexture = loadTextureFromFileToGPU("./media/earth2.jpg");
    sunTexture = loadTextureFromFileToGPU("./media/sun2.jpg");
    moonTexture = loadTextureFromFileToGPU("./media/moon.jpg");
    saturnTexture = loadTextureFromFileToGPU("./media/saturn2.jpg");
    ringTexture = loadTextureFromFileToGPU("./media/saturn_ring.jpg");
    
    std::string textureFolderPath = "./media/skyboxDefault"; // Change to "./media/skybox" to get a nebulae skybox
    std::string textureExtension = ".png";

    std::vector<std::string> faces = {
        textureFolderPath + "/right" + textureExtension,
        textureFolderPath + "/left" + textureExtension,
        textureFolderPath + "/top" + textureExtension,
        textureFolderPath + "/bottom" + textureExtension,
        textureFolderPath + "/front" + textureExtension,
        textureFolderPath + "/back" + textureExtension
    };
    cubemapTexture = loadCubemap(faces);
}

void init() {
    initGLFW();
    initOpenGL();
    initCPUgeometry();
    initGPUprogram();
    initGPUgeometry();
    initCamera();
    initTextures();

    glfwSetTime(0.0);
}

void clear() {
    glDeleteProgram(g_program);
    glDeleteProgram(skyboxProgram);

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

    // Draw celestial bodies
    glUseProgram(g_program);

    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix));

    const glm::vec3 camPosition = g_camera.getPosition();
    glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition.x, camPosition.y, camPosition.z);

    glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glUniform3fv(glGetUniformLocation(g_program, "lightPos"), 1, glm::value_ptr(lightPosition));

    // Render the Sun
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glm::mat4 modelMat = g_sun;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix3fv(glGetUniformLocation(g_program, "normalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));
    glUniform3fv(glGetUniformLocation(g_program, "objectColor"), 1, glm::value_ptr(sunColor));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_TRUE);
    g_sphereMesh->render();

    // Render the Earth
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    modelMat = g_earth;
    normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix3fv(glGetUniformLocation(g_program, "normalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_FALSE);
    g_sphereMesh->render();

    // Render the Moon
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, moonTexture);
    modelMat = g_moon;
    normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix3fv(glGetUniformLocation(g_program, "normalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_FALSE);
    g_sphereMesh->render();

    // Render Saturn
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, saturnTexture);
    modelMat = g_saturn;
    normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix3fv(glGetUniformLocation(g_program, "normalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));
    glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_FALSE);
    g_sphereMesh->render();

    // Render Saturn's Rings
    if (g_ringMesh) {
        glDisable(GL_CULL_FACE); // Disable face culling for rings
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ringTexture);
        glm::mat4 ringModelMat = g_saturn;

        // Tilt the rings
        float ringTiltAngle = glm::radians(27.0f);
        ringModelMat = glm::rotate(ringModelMat, ringTiltAngle, glm::vec3(1.0f, 0.0f, 0.0f));

        normalMat = glm::transpose(glm::inverse(glm::mat3(ringModelMat)));
        glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(ringModelMat));
        glUniformMatrix3fv(glGetUniformLocation(g_program, "normalMat"), 1, GL_FALSE, glm::value_ptr(normalMat));
        glUniform3fv(glGetUniformLocation(g_program, "objectColor"), 1, glm::value_ptr(glm::vec3(1.0f))); // Not used when textured
        glUniform1i(glGetUniformLocation(g_program, "useTexture"), GL_TRUE);
        glUniform1i(glGetUniformLocation(g_program, "isSun"), GL_FALSE);
        g_ringMesh->render();
        glEnable(GL_CULL_FACE);
    }

    // Draw skybox
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxProgram);
    glm::mat4 view = glm::mat4(glm::mat3(g_camera.computeViewMatrix())); // Remove translation from the view matrix
    glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projMatrix));

    glBindVertexArray(skyboxMesh->getVao());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    skyboxMesh->render();
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void update(const float currentFrame) {
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Update simulationTime only if not frozen
    if (!isSimulationFrozen) {
        simulationTime += deltaTime;
    }

    // Calculate orbital and rotation angles
    float earthOrbitAngle = (simulationTime / earthOrbitPeriod) * 2.0f * PI;
    float earthRotationAngle = (simulationTime / earthRotationPeriod) * 2.0f * PI;

    float moonOrbitAngle = (simulationTime / moonOrbitPeriod) * 2.0f * PI;
    float moonRotationAngle = (simulationTime / moonRotationPeriod) * 2.0f * PI;

    float saturnOrbitAngle = (simulationTime / saturnOrbitPeriod) * 2.0f * PI;
    float saturnRotationAngle = (simulationTime / saturnRotationPeriod) * 2.0f * PI;

    // Update Earth
    g_earth = glm::rotate(glm::mat4(1.0f), earthOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
              glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth, 0.0f, 0.0f)) *
              glm::rotate(glm::mat4(1.0f), earthRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
              glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));

    // Update Moon
    glm::mat4 moonLocal = glm::rotate(glm::mat4(1.0f), moonOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
                          glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitMoon, 0.0f, 0.0f)) *
                          glm::rotate(glm::mat4(1.0f), moonRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));

    // Apply Earth's transformation to the Moon
    g_moon = g_earth * moonLocal;

    // Update Saturn
    g_saturn = glm::rotate(glm::mat4(1.0f), saturnOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
               glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitSaturn, 0.0f, 0.0f)) *
               glm::rotate(glm::mat4(1.0f), saturnRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(kSizeSaturn));

    // Process camera movement
    doMovement();
}

int main(int argc, char **argv) {
    init();
    while (!glfwWindowShouldClose(g_window)) {
        update(static_cast<float>(glfwGetTime()));
        render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}
