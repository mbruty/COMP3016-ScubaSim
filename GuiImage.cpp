#include "GuiImage.h"
#include "stb_image.h"
#include "packages/imgui-1.89.2/imgui.h"
#include <math.h>

// Image rotation, from https://github.com/ocornut/imgui/issues/1982
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
{
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
static inline ImVec2 ImRotate(const ImVec2& v, float cos_a, float sin_a)
{
    return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a);
}
void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    ImVec2 pos[4] =
    {
        center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
    };
    ImVec2 uvs[4] =
    {
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 0.0f),
        ImVec2(1.0f, 1.0f),
        ImVec2(0.0f, 1.0f)
    };

    draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
}

// Edited from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
GuiImage::GuiImage(std::string filename, int posX, int posY) {
    unsigned char* image_data = stbi_load(filename.c_str(), &width, &height, NULL, 4);
    if (image_data == NULL)
        throw std::exception("Image was null");

    // Create a OpenGL texture identifier
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    this->posX = posX;
    this->posY = posY;
}

void GuiImage::draw() {
    ImGui::SetCursorPos(ImVec2(posX, posY));

    if (rotationAngle != 0.0f) {
        ImageRotated((void*)ID, ImVec2((width / 2) + posX, (height / 2) + posY), ImVec2(width, height), rotationAngle);
    }
    else {
        bool t = true;
        ImGui::Image((void*)(intptr_t)ID, ImVec2(width, height));
    }
}