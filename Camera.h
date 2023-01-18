// Adapted from https://learnopengl.com/Getting-started/Camera

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <GL/glew.h>
#include "Shader.h"
#include "iostream"
#include "GLFW/glfw3.h"
#include "GuiImage.h"
#include "packages/irrKlang-1.6.0/include/irrKlang.h"
#include <random>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 15.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 1.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
private:
    void generateRandomNeutralPct();

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    double lastX;
    double lastY;
    float neutrallyBuoyantPct = 50.0f;
    float bcdPctFull = 100.0f;
    float tankPctFull = 100.0f;
    float floorDepth = 20 * 10; // Max depth of 20m in cm

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    void setRotation(GuiImage* image);

    
    void setFloorDepth(float depth);
    float getFloorY();
    void updateBuoyancy(GLFWwindow* window, float deltaTime, irrklang::ISound* inflate, irrklang::ISound* deflate, irrklang::ISound* dump);
    void updateBcd(float previousY);
    void setPosition(Shader* shader, std::string loc);
    void setAngle(Shader* shader, std::string loc);
    glm::mat4 getViewMatrix();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
};