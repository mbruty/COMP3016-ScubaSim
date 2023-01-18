#include <vector>
#include "ModelLoader.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <glm/gtc/quaternion.hpp>
#include "Base64Decode.h"

// Nicked from https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

ModelLoader::ModelLoader() {

}

ModelLoader::~ModelLoader() {
}

Model* ModelLoader::load(std::string path, ModelType type) {
	std::string fileText;
	storage.clear();
	// Read the entire model file
	std::ifstream modelFile(path);
	std::string buf;
	while (std::getline(modelFile, buf)) {
		fileText += buf + '\n';
	}
	modelFile.close();

	m_file = fileText;

	auto data = getBytes(type);
	Model* m = new Model(fileText);
	m->rawData = data;
	auto nodes = getObjects("nodes");

    std::map<std::string, std::string> nodesToVisit;

    // Remember all of the nodes we have to visit 
    // As some times we'll access a node early due to it being a child of another node
    for (std::string node : nodes) {
        std::string name = getValueFromObject("name", node);
        nodesToVisit[name] = node;
    }

    // Loop through all nodes
    for (std::string node : nodes) {
        std::string name = getValueFromObject("name", node);

        // If the key hasn't been removed from the map, traverse it
        if (nodesToVisit.count(name)) {
            loadNode(node, m, glm::mat4(0.0f), nodesToVisit);
        }
    }

	std::cout << "Unique buffer views accessed " << bufferViewsAccessed.size() << std::endl;
	std::cout << "Unique accessors accessed " << accessorsAccessed.size() << std::endl;

    bufferViewsAccessed.clear();
    accessorsAccessed.clear();
	return m;
}

void ModelLoader::loadNode(std::string node, Model* model, glm::mat4 matrix, std::map<std::string, std::string>& nodesToVisit) {
	// Get the translation if it exsists;
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	auto translationResults = getArrayFromObject("translation", node);

	if (translationResults.size() > 0) {
		// Convert the translation results to a float, and put it into an array
		float translationArray[3];
		for (int i = 0; i < 3; i++) {
			translationArray[i] = atof(translationResults[i].c_str());
		}
		translation = glm::vec3(translationArray[0], translationArray[1], translationArray[2]);
	}

	// Get the rotation if it exsists
	glm::quat  rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	auto rotationResults = getArrayFromObject("rotation", node);

	if (rotationResults.size() > 0) {
		// Convert the string values to floats and store in a vec4
		float rotationArray[4];
		for (int i = 0; i < 4; i++) {
			rotationArray[i] = atof(rotationResults[i].c_str());
		}
		rotation = glm::quat(
			rotationArray[0],
			rotationArray[1],
			rotationArray[2],
			rotationArray[3]
		);
	}

	// Get the scale if it's avalible
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	auto scaleResults = getArrayFromObject("scale", node);

	if (scaleResults.size() > 0) {
		// Convert the scale results to a float, and put it into an array
		float scaleArray[3];
		for (int i = 0; i < 3; i++) {
			scaleArray[i] = atof(scaleResults[i].c_str());
		}
		scale = glm::vec3(scaleArray[0], scaleArray[1], scaleArray[2]);
	}

	// Create identity matricies for translation, rotation and scale
	glm::mat4 translationMatrix = glm::mat4(1.0f);
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	glm::mat4 scaleMatrix = glm::mat4(1.0f);

	// Update the matricies
	translationMatrix = glm::translate(translationMatrix, translation);
	rotationMatrix = glm::mat4_cast(rotation);
	scaleMatrix = glm::scale(scaleMatrix, scale);

	glm::mat4 calculatedMatrix = matrix * translationMatrix * rotationMatrix * scaleMatrix;

	// If the node contains a mesh, load it
	if (node.find("mesh") != std::string::npos) {
		std::string meshStr = getValueFromObject("mesh", node);

		if (meshStr != "") {
			model->translationsMeshes.push_back(translation);
			model->rotationsMeshes.push_back(rotation);
			model->scaleMeshes.push_back(scale);
			model->matricesMeshes.push_back(calculatedMatrix);

			int mesh = atoi(meshStr.c_str());
			loadMesh(mesh, model);
		}
	}

    // Remove this node from the ones to visit
    std::string name = getValueFromObject("name", node);
    nodesToVisit.erase(name);

    // If the node has any children, visit them with the calculated matrix
    if (node.find("children") != std::string::npos) {
        auto children = getArrayFromObject("children", node);

        for (auto child : children) {
            loadNode(child, model, calculatedMatrix, nodesToVisit);
        }
    }

}

void ModelLoader::loadMesh(int meshIdx, Model* model) {
	std::string mesh = getObject("meshes", meshIdx);

	auto primitives = getSubObjects("primitives", mesh);

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textures;
	std::vector<GLuint> indices;
	std::vector<Vertex> vertices;
	for (auto& primitive : primitives) {
		int positionAccessorIdx = getIntValueFromObject("POSITION", primitive);
		int normalAccessorIdx = getIntValueFromObject("NORMAL", primitive);
		int textureAccessorIdx = getIntValueFromObject("TEXCOORD_0", primitive);
		int indicesAccessorIdx = getIntValueFromObject("indices", primitive);

		accessorsAccessed.insert(positionAccessorIdx);
		accessorsAccessed.insert(normalAccessorIdx);
		accessorsAccessed.insert(textureAccessorIdx);
		accessorsAccessed.insert(indicesAccessorIdx);

		// Position
		std::string posResult = getObject("accessors", positionAccessorIdx);
		std::vector<float> posVector = getFloatsFromAccessor(posResult, model);
		groupFloatsToVector3(posVector, &positions);

		// Normals
		std::string normalResult = getObject("accessors", normalAccessorIdx);
		std::vector<float> normalVector = getFloatsFromAccessor(normalResult, model);
		groupFloatsToVector3(normalVector, &normals);

		// Textures
		std::string textureResult = getObject("accessors", textureAccessorIdx);
		std::vector<float> textureVector = getFloatsFromAccessor(textureResult, model);
		groupFloatsToVector2(normalVector, &textures);

		// Colours
		int materialIdx = getIntValueFromObject("material", mesh);
		std::string material = getObject("materials", materialIdx);
		std::vector<std::string> colourFactor = getArrayFromObject("baseColorFactor", material);
		glm::vec4 colour;
		if (colourFactor.size() == 0) {
			colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
        else {
            colour = glm::vec4(atof(colourFactor[0].c_str()), atof(colourFactor[1].c_str()), atof(colourFactor[2].c_str()), atof(colourFactor[3].c_str()));
        }

		// Indicies
		std::string indicesStr = getObject("accessors", indicesAccessorIdx);
		std::vector<GLuint> ind = createIndicies(indicesStr, model);
		indices.insert(indices.end(), ind.begin(), ind.end());

		createVerticies(positions, normals, colour, textures, &vertices);
	}

	std::vector<Texture> textureVec;
	Mesh* m = new Mesh(vertices, indices, textureVec);

	model->meshes.push_back(m);
}

void ModelLoader::createVerticies(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	glm::vec4 colour,
	std::vector<glm::vec2> textures,
	std::vector<Vertex>* vertices
) {

	for (int i = 0; i < positions.size(); i++)
	{
		vertices->push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				colour,
				textures[i]
			}
		);
	}
}

std::vector<GLuint> ModelLoader::createIndicies(std::string accessor, Model* model) {
	std::vector<GLuint> result;
	int bufferViewIdx = getIntValueFromObject("bufferView", accessor);
	bufferViewsAccessed.insert(bufferViewIdx);
	int count = getIntValueFromObject("count", accessor);
	int accessorByteOffset = getIntValueFromObject("byteOffset", accessor);
	int componentType = getIntValueFromObject("componentType", accessor);

	std::string bufferView = getObject("bufferViews", bufferViewIdx);
	int byteOffset = getIntValueFromObject("byteOffset", bufferView);

	unsigned int beginning = byteOffset + accessorByteOffset;
	if (componentType == 5125) {
		for (unsigned int i = beginning; i < byteOffset + accessorByteOffset + count * 4; i) {
			unsigned char bytes[] = { model->rawData[i++], model->rawData[i++], model->rawData[i++], model->rawData[i++] };
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			result.push_back((GLuint)value);
		}
	}

	else if (componentType == 5123) {
		for (unsigned int i = beginning; i < byteOffset + accessorByteOffset + count * 2; i) {
			unsigned char bytes[] = { model->rawData[i++], model->rawData[i++] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			result.push_back((GLuint)value);
		}
	}

	else if (componentType == 5122) {
		for (unsigned int i = beginning; i < byteOffset + accessorByteOffset + count * 2; i) {
			unsigned char bytes[] = { model->rawData[i++], model->rawData[i++] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			result.push_back((GLuint)value);
		}
	}

	return result;
}


std::vector<float> ModelLoader::getFloatsFromAccessor(std::string accessor, Model* model) {
	std::vector<float> result;

	int bufferViewIdx = getIntValueFromObject("bufferView", accessor);
	bufferViewsAccessed.insert(bufferViewIdx);
	int count = getIntValueFromObject("count", accessor);
	// Sometimes an accessor will have a byte offset, if it cannot be found it'll be defaulted to 0
	int accessorByteOffset = getIntValueFromObject("byteOffset", accessor);
	
	std::string bufferView = getObject("bufferViews", bufferViewIdx);
	int byteOffset = getIntValueFromObject("byteOffset", bufferView);

	std::string type = getValueFromObject("type", accessor);

	int numVerticies;
	if (type == "\"SCALAR\"") numVerticies = 1;
	else if (type == "\"VEC2\"") numVerticies = 2;
	else if (type == "\"VEC3\"") numVerticies = 3;
	else if (type == "\"VEC4\"") numVerticies = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	int beginOffset = byteOffset + accessorByteOffset;
	int dataLength = count * 4 * numVerticies;

	for (int i = beginOffset; i < beginOffset + dataLength;) {
		unsigned char bytes[] = { model->rawData[i++], model->rawData[i++], model->rawData[i++], model->rawData[i++] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		result.push_back(value);
	}
	return result;
}

std::vector<unsigned char> ModelLoader::getBytes(ModelType type) {
	auto buffers = getObject("buffers", 0);
	auto uri = getValueFromObject("uri", buffers);
	if (type == EMBEDDED) {
		std::string bytes;
		std::stringstream ss(uri);
		std::string buf;
		// Get everything after the first comma
		std::getline(ss, buf, ',');
		std::getline(ss, buf, ',');

		return base64_decode(buf);
	}

	else if (type == BINARY) {
		std::string fileName = uri.substr(0, uri.find_last_of('/') + 1);
		std::ifstream in(("data/" + fileName).c_str(), std::ios::binary);
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();

		std::vector<unsigned char> data(contents.begin(), contents.end());
		return data;
	}

	else throw std::invalid_argument("Invalid gltf model type");
}

std::string ModelLoader::getObject(std::string key, int idx) {
	auto data = storage[key];
	if (data.size() == 0) {
		data = getObjects(key);
	}

	return data[idx];
}


std::vector<std::string> ModelLoader::getObjects(std::string key) {
	std::string originalKey = key;
	std::string regexStr = R"(")" + key + R"("\s?:\s?\[)";
	std::regex re(regexStr);
	std::vector<std::string> result;
	std::stringstream ss(m_file);
	std::string currentObjectText;
	bool captureObjects = false;

	int arrayDepth = 0;
	int objectDepth = 0;
	for (std::string buf; std::getline(ss, buf);) {
		currentObjectText += buf + '\n';

		if (std::regex_search(buf, re)) {
			captureObjects = true;
            currentObjectText = "";
		}

		if (buf.find('[') != std::string::npos) {
			arrayDepth++;
		}

		else if (buf.find(']') != std::string::npos) {
			arrayDepth--;
			if (arrayDepth == 0)  captureObjects = false;
		}

		if (buf.find('{') != std::string::npos) {
			if (objectDepth == 1) {
				// The opening of an object
				currentObjectText = "";
			}

			objectDepth++;
		}

		// End of an object
		else if (buf.find('}') != std::string::npos) {
			objectDepth--;
			//
			if (objectDepth == 1) {
				if (currentObjectText != "" && captureObjects) {
					result.push_back(currentObjectText);
				}

				currentObjectText = "";
			}
		}
	}

	storage[originalKey] = result;

	return result;
}

std::vector<std::string> ModelLoader::getSubObjects(std::string key, std::string object) {
	std::vector<std::string> result;

    std::string regexStr = R"(")" + key + R"("\s?:\s?\[)";
    std::regex re(regexStr);

	std::stringstream ss(object);

	int arrayCount = 0; // Track nested arrays so we only close on the correct array
	int objectCount = 0; // Track how nested we are in objects
	bool collectObjects = false;
	std::string objectText;

	for (std::string buf; std::getline(ss, buf);) {
		if (collectObjects) {
			objectText += buf + '\n';
		}
		ltrim(buf);

		if (collectObjects && buf.find('[') != std::string::npos) {
			arrayCount++;
		}

		else if (collectObjects && buf.find(']') != std::string::npos) {
			// If the array count is already 0, and the line contains a closing bracket, we're at the end of the array
			if (arrayCount == 0) break;
			arrayCount--;
		}

		else if (collectObjects && buf.find('{') != std::string::npos) {
			objectCount++;
		}

		else if (collectObjects && buf.find('}') != std::string::npos) {
			if (objectCount == 1) {
				result.push_back(objectText);
				objectText = "";
			}
			objectCount--;
		}

		if (std::regex_search(buf, re)) {
			collectObjects = true;
		}
	}

	return result;
}

std::string ModelLoader::getValueFromObject(std::string key, std::string object) {
	std::stringstream ss(object);
	for (std::string buf; std::getline(ss, buf); ) {
		if (buf.find(key) != std::string::npos) {

			// get the bit after the :
			std::stringstream ss2(buf);
			std::string buf2;
			std::string result;
			std::getline(ss2, buf2, ':');
			std::getline(ss2, buf2, ':');

			result = buf2;
			while (std::getline(ss2, buf2, ':')) {
				result += buf2;
			}

			ltrim(buf2);

			// If the line contains a comma, remove it
			if (buf2.find(',') != std::string::npos) {
				buf2.pop_back();
			}

			return buf2;
		}
	}
	return "";
}

int ModelLoader::getIntValueFromObject(std::string key, std::string object) {
	std::string result = getValueFromObject(key, object);

	if (result != "") {
		return atoi(result.c_str());
	}

	return 0;
}

std::vector<std::string> ModelLoader::getArrayFromObject(std::string key, std::string object) {
	std::stringstream ss(object);
	std::vector<std::string> result;
	// Move the pointer along until we find the key
	for (std::string buf; std::getline(ss, buf);) {
		// if we find the key, break the loop
		if (buf.find(key) != std::string::npos) break;
	}

	// If the stringstream reached the end & didn't find anything, return an empty vector
	if (ss.eof()) {
		return result;
	}

	// else get all the values
	for (std::string buf; std::getline(ss, buf);) {
		// If we're at the end of an array, return the result
		if (buf.find(']') != std::string::npos) return result;

		ltrim(buf);

		// If the line contains a comma, remove it
		if (buf.find(',') != std::string::npos) {
			buf.pop_back();
		}

		result.push_back(buf);
	}

	return result;
}

std::vector<glm::vec4> ModelLoader::groupFloatsToVector4(std::vector<float> vec) {
	std::vector<glm::vec4> result;

	for (int i = 0; i < vec.size();) {
		glm::vec4 vec4 = glm::vec4(vec[i++], vec[i++], vec[i++], vec[i++]);
		result.push_back(vec4);
	}

	return result;
}

void ModelLoader::groupFloatsToVector3(std::vector<float> vec, std::vector<glm::vec3>* resVec) {
	for (int i = 0; i < vec.size();) {
		glm::vec3 vec3 = glm::vec3(vec[i++], vec[i++], vec[i++]);
		resVec->push_back(vec3);
	}
}

void ModelLoader::groupFloatsToVector2(std::vector<float> vec, std::vector<glm::vec2>* resVec) {
	for (int i = 0; i < vec.size() - 2;) {
		glm::vec2 vec2 = glm::vec2(vec[i++], vec[i++]);
		resVec->push_back(vec2);
	}
}