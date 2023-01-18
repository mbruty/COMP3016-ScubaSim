#pragma once

#include "Game.h"
#include"Shader.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Texture.h"
#include <GLFW/glfw3.h>
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "GuiImage.h"
#include "packages/irrKlang-1.6.0/include/irrKlang.h"
#include "packages/imgui-1.89.2/imgui.h"

enum Shaders { Base, Blur, FrameBuffer, NumShaders = 3 };
enum Fonts { BaseFont, Time, NumFonts = 2 };
enum Sounds { UnderwaterAmbient, Inflate, Deflate, Dump, NumSounds = 4 };
class Game
{
private:
    // Default to 1080p
	unsigned int WIDTH = 1920;
	unsigned int HEIGHT = 1080;
	GLFWwindow* m_window;
	Shader* m_shaders[NumShaders];
	Mesh* m_mesh;
	std::vector<Model*> m_models;
    Model* m_backgroundModel;
	Camera* m_camera;
	GLuint colorBuffers[2];
	GLuint pingpongFBO[2];
	GLuint pingpongBuf[2];
	GLuint bloomTexture;
	GLuint postProcessingTexture;
	GLuint postProcessingFBO;

    std::vector<GuiImage*> m_images;
    ImFont* m_fonts[NumFonts];

    irrklang::ISoundEngine* engine;
    std::vector<irrklang::ISound*> m_sounds;

	bool torchIsOn = false;
	bool fLock = false;

	float m_rotation = 0.0f;
	double m_prevTime = glfwGetTime();
	double previousX;
	double previousY;
    double timeEnteredWater = 0.0;
    void drawUiBars();
	void setupFrameBuffers();
	void setupShaders();
	void loadModels();
    void setupImGui();
    void setupAudio();
	bool update();
	bool display();
public:
	Game();
	~Game();
	int start();
};

