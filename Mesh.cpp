#include "Mesh.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture> textures)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;
	m_vao = new VAO();
	m_vao->bind();

	// Generates Vertex Buffer Object and links it to vertices
	m_vbo = new VBO(vertices);

	// Generates Element Buffer Object and links it to indices
	if (indices.size() > 0)
		m_ebo = new EBO(indices);

	m_vao->link(m_vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	m_vao->link(m_vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	m_vao->link(m_vbo, 2, 4, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	m_vao->link(m_vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(10 * sizeof(float)));

	// Unbind all to prevent accidentally modifying them
	m_vao->unbind();
	m_vbo->unbind();
	if (indices.size() > 0)
		m_ebo->unbind();

	count = indices.size();
}

Mesh::~Mesh() {
	delete m_vao;
	delete m_vbo;
	if (vertices.size() > 0)
		delete m_ebo;
}

void Mesh::Draw(Shader* shader) {
	m_vao->bind();

	if (textures.size() > 0) {
		textures[0].texUnit(shader, "texture1", 0);
		textures[0].bind();
	}

	// GLTF indexed based drawing
	if (count > 0) {
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	}

	// OBJ non indexed based drawing
	else {
		m_vbo->bind();
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() * 3);
		m_vbo->unbind();

	}
}

