#pragma once
#include <GL/glew.h>
#include "Vertex.h"
#include <vector>
class VBO {
public:
	GLuint ID;
	VBO(std::vector<Vertex>& vertices);

	void bind();
	void unbind();

	~VBO();
};

