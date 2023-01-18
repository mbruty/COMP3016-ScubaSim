#pragma once
#include <GL/glew.h>
#include "VBO.h"
class VAO {
public:
	GLuint ID;
	VAO();

	void link(VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void bind();
	void unbind();

	~VAO();
};

