#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include "Mesh.h"

Model* ObjLoader::load(std::string path) {
	Model* result = new Model();

	// Read the entire model file
	std::ifstream modelFile(path);
	std::string buf;

	bool firstObject = true;
	std::vector<Vertex> verticies;
	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::vec3> vertexNormals;
	std::vector<GLuint> vertexPositionIndicies;
	std::vector<GLuint> vertexTextureIndicies;
	std::vector<GLuint> vertexNormalIndicies;
	std::vector<GLuint> indicies;
	GLuint index = 0;
	std::vector<Mesh> meshes;
    std::vector<std::string> currentMaterial;

	while (std::getline(modelFile, buf)) {
		// If the line is the start of an object, start a new mesh
		if (buf[0] == 'o') {
			if (firstObject) {
				firstObject = false;
			}
			else {
			}
		}

		// Load the material file
		if (buf[0] == 'm') {
			if (buf.find("mtllib") != std::string::npos) {
				buf.erase(0, buf.find_first_of(' ') + 1);
				std::string filePath = "data/" + buf;
				loadMtl(filePath);
			}
		}

        if (buf[0] == 'u') {
            if (buf.find("usemtl") != std::string::npos) {
                std::string material = buf.erase(0, buf.find_first_of(' ') + 1);
                currentMaterial = materials[material];
            }
        }

		// If we're dealing with a vertex
		else if (buf[0] == 'v' && buf[1] == ' ') {
			vertexPositions.push_back(getVec3FromLine(buf, 1.0f));
		}

		else if (buf[0] == 'v' && buf[1] == 't') {
			textureCoords.push_back(getVec2FromLine(buf, 0.0f));
		}

		else if (buf[0] == 'v' && buf[1] == 'n') {
			vertexNormals.push_back(getVec3FromLine(buf, 0.0f));
		}

		else if (buf[0] == 'f') {
			std::vector<glm::vec3> faces = getFaceFromLine(buf);
			for (int i = 0; i < 3; i++) {
				Vertex v;
				glm::vec3 face = faces[i];

                vertexPositionIndicies.push_back(face[0] - 1);
                vertexTextureIndicies.push_back(face[1] - 1);
                vertexNormalIndicies.push_back(face[2] - 1);
			}
		}
	}

    verticies.resize(vertexPositionIndicies.size(), Vertex());

    glm::vec4 currentColour = glm::vec4(1.0f);

    if (currentMaterial.size() > 0) {
        for (std::string line : currentMaterial) {
            if (line[0] == 'K' && line[1] == 'a') {
                line.erase(0, line.find_last_of(' '));
                std::stringstream ss(line);
                std::string buf;
                float nums[3];
                for (int i = 0; i < 3; i++) {
                    getline(ss, buf, ' ');
                    nums[i] = atof(buf.c_str());
                }

                currentColour = glm::vec4(nums[0], nums[1], nums[2], 1.0f);
            }
        }
    }
    for (int i = 0; i < verticies.size(); i++) {
        verticies[i].position = vertexPositions[vertexPositionIndicies[i]];
        verticies[i].normal = vertexNormals[vertexNormalIndicies[i]];
        verticies[i].texture = textureCoords[vertexTextureIndicies[i]];
        verticies[i].colour = currentColour;
    }

	Mesh* m = new Mesh(verticies, indicies, std::vector<Texture>());
	result->meshes.push_back(m);
	return result;
}

glm::vec3 ObjLoader::getColourFromCurrentMtl() {
	glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);

	// If for some reason we're not using a material (probably shouldn't happen)
	// fail gracefully and return white
	if (currentMtlKey == "") return white;

	std::vector<std::string> material = materials[currentMtlKey];

	for (std::string& m : material) {
		if (m[0] == 'K' && m[1] == 'd') {
			// Copy the string so to not acidentally edit it and corrupt the file
			std::string buf = m;
			buf.erase(0, buf.find_first_of(' ') + 1);

			std::stringstream ss(buf);
			float floats[3] = { 1.0f, 1.0f, 1.0f };
			buf = "";
			for (int i = 0; i < 3 && getline(ss, buf, ' '); i++) {
				floats[i] = atof(buf.c_str());
			}

			return glm::vec3(floats[0], floats[1], floats[2]);
		}
	}

	return white;
}

void ObjLoader::loadMtl(std::string filePath) {
	std::ifstream mtlFile(filePath);
	std::string buf;
	
	std::string currentMtlName;
	std::vector<std::string> lines;
	while (std::getline(mtlFile, buf)) {
		if (buf[0] == '#') continue; // Skip over this line if it's a comment

		else if (buf.find("newmtl") != std::string::npos) {
			// Save the previous material
			if (currentMtlName != "") {
				materials[currentMtlName] = lines;
				lines = std::vector<std::string>();
			}
			buf.erase(0, buf.find_first_of(' ') + 1);

			currentMtlName = buf;
		}

		else {
			lines.push_back(buf);
		}
	}

	materials[currentMtlName] = lines;
}

glm::vec3 ObjLoader::getVec3FromLine(std::string line, float defaultTo) {
	line.erase(0, line.find_first_of(' ') + 1);
	std::stringstream ss(line);
	float result[3] = { defaultTo, defaultTo, defaultTo };
	std::string buf;
	for (int i = 0; std::getline(ss, buf, ' ') && i < 3; i++) {
		result[i] = atof(buf.c_str());
	}

	return glm::vec3(result[0], result[1], result[2]);
}

glm::vec2 ObjLoader::getVec2FromLine(std::string line, float defaultTo) {
	line.erase(0, line.find_first_of(' ') + 1);
	std::stringstream ss(line);
	float result[2] = { defaultTo, defaultTo };

	std::string buf;
	for (int i = 0; std::getline(ss, buf, ' ') && i < 3; i++) {
		result[i] = atof(buf.c_str());
	}

	return glm::vec2(result[0], result[1]);
}

std::vector<glm::vec3> ObjLoader::getFaceFromLine(std::string line) {
    std::vector<glm::vec3> result;
	line.erase(0, line.find_first_of(' ') + 1);

	std::stringstream ss(line);
	std::string buf;
	for (int i = 0; i < 3; i++) {
		// Split the line on spaces
		std::getline(ss, buf, ' ');
		std::stringstream ss2(buf);
        float chunk[3];
		for (int j = 0; j < 3; j++) {
			std::string buf2;
			// Split the buffer on '/'
			std::getline(ss2, buf2, '/');
            chunk[j] = atoi(buf2.c_str());
		}
        result.push_back(glm::vec3(chunk[0], chunk[1], chunk[2]));
	}

	return result;
}