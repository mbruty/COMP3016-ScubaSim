#include "Model.h"
#include <glm/gtc/type_ptr.hpp>

Model::Model(std::string fileData) {
	this->fileData = fileData;
}

void Model::draw(Shader* shader) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotation), rotationAxis);
	model = glm::scale(model, scale);

	int mLoc = glGetUniformLocation(shader->ID, "m_matrix");
	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(model));

	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->Draw(shader);
	}
}

Model::~Model() {

}