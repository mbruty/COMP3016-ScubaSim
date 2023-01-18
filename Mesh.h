#pragma once
#include "Camera.h"
#include "Texture.h"
#include "VAO.h"
#include "Vertex.h"
#include <gl/GL.h>
#include <vector>
#include "EBO.h"

class Mesh {
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
	VBO* m_vbo;
	VAO* m_vao;
	EBO* m_ebo;
	GLsizei count;
	bool isObj;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture> textures);
	~Mesh();
	void Draw(Shader* shader);
};