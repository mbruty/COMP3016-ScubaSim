#pragma once
#include "Shader.h"
#include "Model.h"
#include <map>
#include <set>

enum ModelType {
	EMBEDDED,
	BINARY
};

class ModelLoader {
private:
	std::string m_file;
	std::vector<std::string> getObjects(std::string key);
	std::string getObject(std::string key, int idx);
	std::vector<std::string> getSubObjects(std::string key, std::string object);
	std::vector<unsigned char> getBytes(ModelType type);
	std::string getValueFromObject(std::string key, std::string object);
	int getIntValueFromObject(std::string key, std::string object);
	std::vector<std::string> getArrayFromObject(std::string key, std::string object);
	std::map<std::string, std::vector<std::string>> storage;
	void loadMesh(int meshIdx, Model* model);
	std::vector<float> getFloatsFromAccessor(std::string accessor, Model* model);
	std::vector<GLuint> createIndicies(std::string accessor, Model* model);
	std::set<int> bufferViewsAccessed;
	std::set<int> accessorsAccessed;
	void createVerticies(std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, glm::vec4 colour, std::vector<glm::vec2> textures, std::vector<Vertex>* vertices);

public:
	void groupFloatsToVector2(std::vector<float> vec, std::vector<glm::vec2>* resVec);
	void groupFloatsToVector3(std::vector<float> vec, std::vector<glm::vec3>* resVec);
	std::vector<glm::vec4> groupFloatsToVector4(std::vector<float> vec);
	ModelLoader();
    void loadNode(std::string node, Model* model, glm::mat4 matrix, std::map<std::string, std::string>& nodesToVisit);

	Model* load(std::string path, ModelType type);
	~ModelLoader();
};