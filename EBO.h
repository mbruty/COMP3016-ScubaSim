#pragma once
#include <GL/glew.h>
#include <vector>
class EBO {
public:
	GLuint ID;
	EBO(std::vector<GLuint>& indices);

	void bind();
	void unbind();

	~EBO();
};

