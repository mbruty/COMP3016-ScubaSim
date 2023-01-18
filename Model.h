#pragma once
#include <glm/fwd.hpp>
#include "Shader.h";
#include "Mesh.h"
#include <glm/gtc/quaternion.hpp>

class Model {
public:
	std::vector<Mesh*> meshes;
	std::vector<glm::vec3> translationsMeshes;
	std::vector<glm::quat> rotationsMeshes;
	std::vector<glm::vec3> scaleMeshes;
	std::vector<glm::mat4> matricesMeshes;
	glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	float rotation = 0.0f;
	glm::vec3 position = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	std::vector<unsigned char> rawData;
	std::string fileData;
	Model() {}
	Model(std::string fileData);
	void draw(Shader* shader);
	~Model();
};