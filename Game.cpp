#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp> //includes GLM
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glm/ext/matrix_transform.hpp> // GLM: translate, rotate
#include <glm/ext/matrix_clip_space.hpp> // GLM: perspective and ortho 
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <sstream>
#include "packages/imgui-1.89.2/imgui.h"
#include "packages/imgui-1.89.2/imgui_impl_glfw.h"
#include "packages/imgui-1.89.2/imgui_impl_opengl3.h"
#include "Game.h"
#include "ModelLoader.h"
#include <iomanip>
#include "ObjLoader.h"
#include <math.h>

// Rectangle to cover the whole screen
float rectangleVertices[] =
{
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};

GLuint rectangleVao;

float gamma = 2.2f;

/// <summary>
/// Draws a blue, rounded bar at the coordinates
/// </summary>
/// <param name="x">x position on screen</param>
/// <param name="y">y position on screen</param>
/// <param name="w">width of the bar</param>
/// <param name="h">height of the bar</param>
void drawBar(int x, int y, int w, int h) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(ImVec2(x, y - 1), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(0.0 / 255.0, 255 / 240.0, 255 / 255.0, 255 / 255.0)), 30.0f);
}

/// <summary>
/// Updates the programs state
/// </summary>
/// <returns>True if successful, false if not</returns>
bool Game::update() {
	double crntTime = glfwGetTime();
	double deltaTime = crntTime - m_prevTime;
	float posYBeforeUpdate = m_camera->Position.y;
	m_prevTime = crntTime;

    // Handle keyboard inputs
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera->ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_camera->ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		m_camera->ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(m_window, GL_TRUE);
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
        // Update the positions
        m_camera->floorDepth -= 0.5f;

        // Clamp the floor depth at 5m
        if (m_camera->floorDepth < 50.0f)
            m_camera->floorDepth = 50.0f;
        m_models[1]->position.y = m_camera->getFloorY() - 1;
        m_models[2]->position.y = m_camera->getFloorY() - 20;
        m_models[3]->position.y = m_camera->getFloorY() + 40;
    }
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        // Update the positions
        m_camera->floorDepth += 0.5f;

        // Clamp the floor depth at 100m
        if (m_camera->floorDepth > 1000.0f)
            m_camera->floorDepth = 1000.0f;
        m_models[1]->position.y = m_camera->getFloorY() - 1;
        m_models[2]->position.y = m_camera->getFloorY() - 20;
        m_models[3]->position.y = m_camera->getFloorY() + 40;
    }
	if (glfwGetKey(m_window, GLFW_KEY_F) == GLFW_PRESS && !fLock) {
        engine->play2D("media/flashlight.mp3");
		torchIsOn = !torchIsOn;
		GLuint torchLoc = glGetUniformLocation(m_shaders[Base]->ID, "torchIsOn");
		glUniform1i(torchLoc, torchIsOn);
		fLock = true;
	}
	else if (glfwGetKey(m_window, GLFW_KEY_F) != GLFW_PRESS) {
		fLock = false;
	}

    // Handle mouse movement
	double xpos, ypos, deltaX, deltaY;
	glfwGetCursorPos(m_window, &xpos, &ypos);

	deltaX = xpos - previousX;
	deltaY = previousY - ypos;

	m_camera->processMouseMovement(deltaX, deltaY);
    m_camera->updateBuoyancy(m_window, deltaTime, m_sounds[Inflate], m_sounds[Deflate], m_sounds[Dump]);
    
	previousX = xpos;
	previousY = ypos;

	m_camera->updateBcd(posYBeforeUpdate);

    m_camera->setRotation(m_images[1]);


    // Update the jellyfish light
    // Calculate new rotation
    const float rotationAngle = 0.1f;
    float s = sin(glm::radians(rotationAngle));
    float c = cos(glm::radians(rotationAngle));
    glm::vec3& jellyPos = m_models[3]->position;

    m_models[3]->rotation -= rotationAngle;

    float xnew = jellyPos.x * c - jellyPos.z * s;
    float znew = jellyPos.x * s + jellyPos.z * c;

    jellyPos.x = xnew;
    jellyPos.z = znew;

    // Send the new position to the shader
    GLuint jellyLoc = glGetUniformLocation(m_shaders[Base]->ID, "jellyfishPos");
    glUniform3f(jellyLoc, jellyPos.x, jellyPos.y, jellyPos.z);

    // Calculate when the player submurged
    // If the player is submurged, play the underwater breathing sound
    if (m_camera->Position.y >= 0) {
        timeEnteredWater = 0;
        m_sounds[UnderwaterAmbient]->setIsPaused(true);
    }
    else if (timeEnteredWater == 0) {
        timeEnteredWater = crntTime;
        m_sounds[UnderwaterAmbient]->setIsPaused(false);
    }

    // Set the background cylendar's position to the player's
    m_backgroundModel->position.x = m_camera->Position.x;
    m_backgroundModel->position.z = m_camera->Position.z;

	return true;
}

/// <summary>
/// Draw's the scene, applies post processing effects, and draws GUI ontop
/// </summary>
/// <returns>True if successful, fasle if not</returns>
bool Game::display() {
	// Draw everyhing to the post processing frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);

    // Set the background colour to sky blue
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	m_shaders[Base]->use();
	glClear(GL_COLOR_BUFFER_BIT);

    // Tell the shader where the camera is, and where it's looking
	m_camera->setPosition(m_shaders[Base], "camPos");
	m_camera->setAngle(m_shaders[Base], "camDirection");

	// creating the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = m_camera->getViewMatrix();

	int vLoc = glGetUniformLocation(m_shaders[Base]->ID, "v_matrix");
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(view));

    // For the background cylendar, ignore distance from camera, and flashlight
    GLuint ignoreLoc = glGetUniformLocation(m_shaders[Base]->ID, "ignoreDistFromCamera");
    glUniform1i(ignoreLoc, true);
    m_backgroundModel->draw(m_shaders[0]);
    glUniform1i(ignoreLoc, false);

    // Draw all of the models
    m_shaders[0]->use();
	for (Model* m : m_models) {
		m->draw(m_shaders[0]);
	}

    /* Blurring interperted from https ://learnopengl.com/Advanced-Lighting/Bloom
    * Then modified to give a depth of field effect.
    * The first-pass gets rendered to two frames.
    * The first frame is the entire scene, the second frame is only the items within 5m of the camera
    * 
    * In the below loop, the entire scene is then blurred.
    * After that, the two frames are added together in this process:
    *   if the second frame pixel at the current location isn't black, use the second frame picel
    *   else use the first (blurry) frame pixel
    */

	// Bounce the image data around to blur multiple times
	bool horizontal = true, first_iteration = true;
	// Amount of time to bounce the blur
	int amount = 2;
	m_shaders[Blur]->use();

	for (int i = 0; i < 6; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		glUniform1i(glGetUniformLocation(m_shaders[Blur]->ID, "horizontal"), horizontal);
		glBindTexture(
			GL_TEXTURE_2D, first_iteration ? postProcessingTexture : pingpongBuf[!horizontal]
		);

		glBindVertexArray(rectangleVao);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		horizontal = !horizontal;

		first_iteration = false;
	}

	// Switch back to the default frame buffer
	// and display the post processing effects on the rectangle 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_shaders[FrameBuffer]->use();
	glBindVertexArray(rectangleVao);
	glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pingpongBuf[!horizontal]);

	glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw GUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    bool t = true;

    ImGui::Begin("window", &t, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos(ImVec2(0, 0));


    // Draw all the images
    for (GuiImage* image : m_images) {
        image->draw();
    }

    // Draw the stats
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(30, 30, 30, 255));
    ImGui::PushFont(m_fonts[BaseFont]);
    std::stringstream depth;
    depth << std::fixed << std::setprecision(1) << ( - m_camera->Position.y / 10);
    std::string text = "Depth: " + depth.str() + "m";
    ImGui::SetCursorPos(ImVec2(130.0f, 315.0f));
    ImGui::Text(text.c_str());

    ImGui::SetCursorPos(ImVec2(170.0f, 180.0f));
    ImGui::Text("Dive Time");

    ImGui::PopFont();

    ImGui::PushFont(m_fonts[Time]);
    ImGui::SetCursorPos(ImVec2(180.0f, 215.0f));
    std::string timeText = "0:00";
    int secs = (int)(glfwGetTime() - timeEnteredWater);
    if (timeEnteredWater != 0 && secs != 0) {
        // Calculate mins passed
        int mins = secs / 60;
        
        // Calculate seconds after mins is taken away
        secs -= 60 * mins;
        timeText = std::to_string(secs);

        if (secs < 10)
            timeText = "0" + timeText;

        timeText = std::to_string(mins) + ":" + timeText;
    }
    ImGui::Text(timeText.c_str());
    ImGui::PopFont();


    ImGui::PopStyleColor();

    ImGui::PushFont(m_fonts[BaseFont]);
    std::stringstream floordepth;
    floordepth << std::fixed << std::setprecision(1) << (-m_camera->floorDepth / 10);
    std::string floordepthtxt = "Floor: " + floordepth.str() + "m";
    ImGui::SetCursorPos(ImVec2(10.0f, 510.0f));
    ImGui::Text(floordepthtxt.c_str());
    ImGui::PopFont();

    drawUiBars();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	m_shaders[Base]->use();
	return true;
}

void Game::drawUiBars() {
    float bcdPct = m_camera->bcdPctFull;
    float tankFullPct = m_camera->tankPctFull;
    const int barWidth = 37;
    const int barHeight = 344;

    // Buoyancy bar
    bcdPct /= 100;
    float buoyancyHeight = barHeight * bcdPct;

    // Clamp the value to a minimum of 37 to keep the ui looking nice
    if (buoyancyHeight < 37)
        buoyancyHeight = 37;
    drawBar(20, 87 + (barHeight - buoyancyHeight), barWidth, buoyancyHeight);

    // Air bar
    tankFullPct /= 100;
    float tankHeight = barHeight * tankFullPct;

    // Clamp the value to a minimum of 37 to keep the ui looking nice
    if (tankHeight < 37)
        tankHeight = 37;

    drawBar(463, 87 + (barHeight - tankHeight), barWidth, tankHeight);
}

/// <summary>
/// Sets up framebuffers needed for post processing.
/// Adapted from: https://learnopengl.com/Advanced-OpenGL/Framebuffers
/// </summary>
void Game::setupFrameBuffers() {
	// Creating a custom framebuffer to use post processing effects on
	glGenFramebuffers(1, &postProcessingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);
	// Generate a frame buffer texture for us to fiddle with in the post processing
	glGenTextures(1, &postProcessingTexture);
	glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
	// Create a texture that will cover the entire screen
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	// Clamp it to the screen so that effects don't bleed over edges
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTexture, 0);

	// Creating a second framebuffer for the bloom effect
	glGenTextures(1, &bloomTexture);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);
	// Create a texture that will cover the entire screen
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	// Clamp it to the screen so that effects don't bleed over edges
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTexture, 0);

	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// Create Render Buffer Object
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	// Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Post-Processing Framebuffer error: " << fboStatus << std::endl;


	// Create the pingpong buffers for two-pass blur
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuf);

	// For all of the pingpong buffers, create the texture & clamp it 
	// the same process as done above
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuf);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuf[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuf[i], 0
		);
	}
}

/// <summary>
/// Loads in, compiles, and adds all of the required shaders to their arrays
/// </summary>
void Game::setupShaders() {
	m_shaders[Base] = new Shader("media/shader.vert", "media/shader.frag");
	m_shaders[FrameBuffer] = new Shader("media/framebuffer.vert", "media/framebuffer.frag");
	m_shaders[Blur] = new Shader("media/framebuffer.vert", "media/blur.frag");
	m_shaders[Base]->use();

    // creating the projection matrix
    glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 0.1f, 2000.0f);
    //adding the Uniform to the shader
    int pLoc = glGetUniformLocation(m_shaders[Base]->ID, "p_matrix");
    glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));
	GLuint lightColourLoc = glGetUniformLocation(m_shaders[Base]->ID, "lightColour");
	glUniform3f(lightColourLoc, 0.988f, 0.922f, 0.702f);
	GLuint torchLoc = glGetUniformLocation(m_shaders[Base]->ID, "torchIsOn");
	glUniform1i(torchLoc, torchIsOn);

    GLuint jellyAmbientLoc = glGetUniformLocation(m_shaders[Base]->ID, "jellyfishAmbient");
    glUniform3f(jellyAmbientLoc, 0.894, 0.51, 0.851);

	GLuint lightAmbientLoc = glGetUniformLocation(m_shaders[Base]->ID, "ambient");
	glUniform3f(lightAmbientLoc, 0.529, 0.808, 0.922);

    GLuint ignoreLoc = glGetUniformLocation(m_shaders[Base]->ID, "ignoreDistFromCamera");
    glUniform1i(ignoreLoc, false);

	m_shaders[FrameBuffer]->use();
	// Tell the framebuffer to use the first texture
	GLuint screenLoc = glGetUniformLocation(m_shaders[FrameBuffer]->ID, "infocus");
	glUniform1i(screenLoc, 0);
	GLuint blurLoc = glGetUniformLocation(m_shaders[FrameBuffer]->ID, "blured");
	glUniform1i(screenLoc, 1);
	GLuint gammaLoc = glGetUniformLocation(m_shaders[FrameBuffer]->ID, "gamma");
	glUniform1f(gammaLoc, gamma);
	m_shaders[Blur]->use();
	// Tell the blur program what texture to use
	GLuint imageLoc = glGetUniformLocation(m_shaders[Blur]->ID, "image");
	glUniform1i(imageLoc, 0);
	GLuint samplesLoc = glGetUniformLocation(m_shaders[Blur]->ID, "samples");
	glUniform1i(samplesLoc, 8);

	// Create the rectangle buffers
    // Required for the post-processing effects
	glGenVertexArrays(1, &rectangleVao);
	GLuint rectangleVbo;
	glGenBuffers(1, &rectangleVbo);
	glBindVertexArray(rectangleVao);
	glBindBuffer(GL_ARRAY_BUFFER, rectangleVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

/// <summary>
/// Loads in all of the required models, and places them into their array
/// </summary>
void Game::loadModels() {
    ObjLoader oLoader;
	ModelLoader loader;
    m_models.push_back(oLoader.load("data/blue.obj"));
	m_models.push_back(loader.load("data/sand.gltf", EMBEDDED));
    m_models.push_back(loader.load("data/boat.gltf", EMBEDDED));
    m_models.push_back(loader.load("data/jellyfish.gltf", EMBEDDED));
    m_backgroundModel = loader.load("data/cylinder.gltf", EMBEDDED);


    // Apply default transformations for the scene
	m_models[0]->scale = glm::vec3(2000, 0.0f, 2000);

	m_models[1]->position.y = m_camera->getFloorY() - 1;
	m_models[1]->scale = glm::vec3(2000, 0.0f, 2000);



	m_models[2]->scale = glm::vec3(0.5f, 0.5f, 0.5f);
	m_models[2]->rotation = -90.0f;
	m_models[2]->rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	m_models[2]->position.y = m_camera->getFloorY() - 20;

    m_models[3]->position.y = m_camera->getFloorY() + 40;
    m_models[3]->position.x = 60;
    m_models[3]->rotation = 90;
    m_models[3]->rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

    m_backgroundModel->scale = glm::vec3(600.0f, 1005.0f, 600.0f);
    m_backgroundModel->position.y = -1004.5f;
}

/// <summary>
/// Initalizes imgui and loads the gui images & fonts
/// </summary>
void Game::setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    // Use opengl 4.5
    ImGui_ImplOpenGL3_Init("#version 450 core");

    // Load the images
    m_images.push_back(new GuiImage("media/computer.png", 10, 10));
    m_images.push_back(new GuiImage("media/compass.png", 10, 10));

    // Load the roboto font, and scale it to 48px & 105px
    ImGuiIO& io = ImGui::GetIO();
    m_fonts[BaseFont] = io.Fonts->AddFontFromFileTTF("media/Roboto-Black.ttf", 48);
    m_fonts[Time] = io.Fonts->AddFontFromFileTTF("media/Roboto-Black.ttf", 105);
}

/// <summary>
/// Sets up irrklang and loads in the audio files
/// </summary>
void Game::setupAudio() {
    m_sounds.resize(NumSounds);
    engine = irrklang::createIrrKlangDevice();

    if (!engine) return;

    m_sounds[UnderwaterAmbient] = engine->play2D("media/underwater.mp3", true, true, false, irrklang::ESM_AUTO_DETECT, true);
    m_sounds[Inflate] = engine->play2D("media/inflate.mp3", true, true, false, irrklang::ESM_AUTO_DETECT, true);
    m_sounds[Deflate] = engine->play2D("media/deflate.mp3", true, true, false, irrklang::ESM_AUTO_DETECT, true);
    m_sounds[Dump] = engine->play2D("media/dump.mp3", true, true, false, irrklang::ESM_AUTO_DETECT, true);

    // Preload the flashlight
    engine->addSoundSourceFromFile("media/flashlight.mp3", irrklang::ESM_AUTO_DETECT, true);
}

Game::Game() {
	// Create the window
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    WIDTH = mode->width;
    HEIGHT = mode->height;

	m_window = glfwCreateWindow(WIDTH, HEIGHT, "ScubaSim", primary, NULL);
	glfwMakeContextCurrent(m_window);
	glewInit();
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    setupImGui();

	m_camera = new Camera();

	setupShaders();

	glEnable(GL_MULTISAMPLE);

	setupFrameBuffers();

	loadModels();

    setupAudio();
}

/// <summary>
/// Deletes all objects from memory and shuts down imgui & irrklang
/// </summary>
Game::~Game() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete m_camera;
	for (int i = 0; i < NumShaders; i++)
		delete m_shaders[i];

	for (int i = 0; i < m_models.size(); i++)
		delete m_models[i];

    for (irrklang::ISound* sound : m_sounds)
        sound->drop();

    engine->drop();
}

// Debug callback
// Copied from a stack overflow thread that I've lost the link to
void GLAPIENTRY MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}


int Game::start() {
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	if (m_window == NULL) {
		std::cout << "Creating window failed";
		glfwTerminate();
		return -1;
	}

	while (!glfwWindowShouldClose(m_window)) {
		if (!update()) {
			break;
		}
		if (!display()) {
			break;
		}
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	glfwDestroyWindow(m_window);
	glfwTerminate();

	return 0;
}