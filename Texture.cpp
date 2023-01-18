// Based off of implementation from learn opengl
// https://learnopengl.com/Getting-started/Textures

#pragma once
#include <iostream>
#include "stb_image.h"
#include "Texture.h"

Texture::Texture(Shader* shader, std::string textureImageFileName, std::string uniformLocation, std::string textureType) {
	shader->use();
	type = textureType;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.
	auto location = "media/textures/" + textureImageFileName;
	unsigned char* data = stbi_load(location.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	auto uniform = glGetUniformLocation(shader->ID, uniformLocation.c_str());
	glUniform1i(uniform, 0);
}

void Texture::texUnit(Shader* shader, std::string uniform, GLuint unit) {
	// Shader needs to be activated before changing the value of a uniform
	shader->use();

	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader->ID, uniform.c_str());
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() {

}

Texture::~Texture() {

}