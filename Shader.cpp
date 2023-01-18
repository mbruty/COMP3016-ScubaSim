// Based off of learn opengl implemention
// https://learnopengl.com/Getting-started/Shaders

#pragma once
#include "Shader.h"
#include "Utilities.h"
#include <iostream>

// Checks if the different Shaders have compiled properly
void compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}

Shader::Shader(std::string vertexFile, std::string fragFile) {
	std::string vertex = read_file(vertexFile);
	std::string frag = read_file(fragFile);

	ID = glCreateProgram();

	const char* vertexSource = vertex.c_str();
	const char* fragSource = frag.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, 0);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, 0);
	glCompileShader(fragShader);
	compileErrors(fragShader, "FRAGMENT");

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

void Shader::use() {
	glUseProgram(ID);
}

Shader::~Shader() {
	glDeleteProgram(ID);
}


