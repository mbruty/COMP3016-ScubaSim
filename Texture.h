#pragma once
#include "Shader.h"
#include <GL/glew.h>

class Texture {
public:
	GLuint ID;
	GLint width, height, nrChannels;
	std::string type;
	Texture(Shader* shader, std::string textureImageFileName, std::string uniformLocation, std::string textureType);

	void bind();
	void texUnit(Shader* shader, std::string uniform, GLuint unit);
	void unbind();

	~Texture();
};

