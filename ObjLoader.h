#pragma once
#include <map>
#include "Model.h"

class ObjLoader {
private:
	glm::vec3 getVec3FromLine(std::string line, float defaultTo);
	glm::vec2 getVec2FromLine(std::string line, float defaultTo);
	void loadMtl(std::string filePath);
    std::vector<glm::vec3> getFaceFromLine(std::string line);
	std::map<std::string, std::vector<std::string>> materials;
	std::string currentMtlKey;
	glm::vec3 getColourFromCurrentMtl();
public:
	Model* load(std::string filePath);
};