#include "Camera.h"

void Camera::generateRandomNeutralPct() {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(25, 75); // define the range

    neutrallyBuoyantPct = distr(gen);
}

void Camera::updateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    glTranslatef(-Front.x, -Front.y, -Front.z);

    // Also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
    generateRandomNeutralPct();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
    generateRandomNeutralPct();
}

void Camera::setRotation(GuiImage* image) {
    float mod = fmod(Yaw, 360.0f);
    image->rotationAngle = glm::radians(-mod);
}

/// <summary>
/// Set the floor depth in M
/// </summary>
void Camera::setFloorDepth(float depth) {
    floorDepth = depth * 10;
}

float Camera::getFloorY() {
    return -floorDepth;
}

void Camera::updateBuoyancy(GLFWwindow* window, float deltaTime, irrklang::ISound* inflate, irrklang::ISound* deflate, irrklang::ISound* dump) {
    float velocity = MovementSpeed * deltaTime;

    // Inflate
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        bcdPctFull += 0.25f;
        tankPctFull -= 0.05f;
        if (bcdPctFull < 100)
            inflate->setIsPaused(false);
        else
            inflate->setIsPaused(true);
    }
    else {
        inflate->setIsPaused(true);
    }

    // Deflate
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        bcdPctFull -= 0.25f;
        if (bcdPctFull > 0)
            deflate->setIsPaused(false);
        else
            deflate->setIsPaused(true);
    }
    else {
        deflate->setIsPaused(true);
    }

    // Dump the air out
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        bcdPctFull -= 0.5f;
        if (bcdPctFull > 0)
            dump->setIsPaused(false);
        else
            dump->setIsPaused(false);
    }
    else {
        dump->setIsPaused(true);
    }

    // Debug to set to neutral
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        bcdPctFull = neutrallyBuoyantPct;
    }

    // Clamp to 100%
    if (bcdPctFull > 100)
        bcdPctFull = 100;

    // Clamp to 0%
    if (bcdPctFull < 0)
        bcdPctFull = 0;

    // Calculate the velocity
    // Max upward / downward velocity is 2 * MovementSpeed
    float normalised = (bcdPctFull - neutrallyBuoyantPct) / (neutrallyBuoyantPct / 2);

    // If you're within 2% of neutral, let you be neutral
    if (bcdPctFull > neutrallyBuoyantPct - 2 && bcdPctFull < neutrallyBuoyantPct + 2) {
        normalised = 0.0f;
    }

    velocity *= normalised;
    Position.y += velocity;

    // Clamp y between 1m and floor
    if (Position.y > 10)
        Position.y = 10;

    if (Position.y < -floorDepth)
        Position.y = -floorDepth;
}

void Camera::updateBcd(float previousY) {
    float deltaY = Position.y - previousY;
    bcdPctFull += deltaY / 10;
}

void Camera::setPosition(Shader* shader, std::string loc) {
    //adding the Uniform to the shader
    int posLoc = glGetUniformLocation(shader->ID, loc.c_str());
    glUniform3f(posLoc, Position.x, Position.y, Position.z);
}


void Camera::setAngle(Shader* shader, std::string loc) {
    //adding the Uniform to the shader
    int posLoc = glGetUniformLocation(shader->ID, loc.c_str());
    glUniform3f(posLoc, Front.x, Front.y, Front.z);
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}