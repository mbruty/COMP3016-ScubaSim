#pragma once
#include <GL/glew.h>
#include <string>
class Shader
{
public:
	GLuint ID;

	Shader(std::string vertexFile, std::string fragFile);

	void use();

	~Shader();
};

