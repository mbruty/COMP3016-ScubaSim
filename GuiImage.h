#pragma once
#include <string>
#include "GL/glew.h"

class GuiImage {
private:
    GLuint ID;
    int width;
    int height;

    int posX;
    int posY;
public:
    float rotationAngle = 0.0f;
    GuiImage(std::string filename, int posX, int posY);
    void draw();
    ~GuiImage();
};