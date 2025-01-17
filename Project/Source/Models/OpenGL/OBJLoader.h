#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
//#include <unordered_set>
#include <iostream>

//#define uset unordered_set

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct Vertex {
	glm::vec3 position;
	glm::vec2 textureCoord;
	glm::vec3 normal;

	static int GetStride() { return (3*2 + 2); }

	inline bool operator==(const Vertex& v2) { return this->position == v2.position && this->textureCoord == v2.textureCoord && this->normal == v2.normal; }
};

// Non-Indexed OBJ Loader
class OBJLoader {

	// Won't use the Vertex struct because these are only the vertex data, not the faces
	std::vector<glm::vec3> positions = std::vector<glm::vec3>();
	std::vector<glm::vec2> textureCoords = std::vector<glm::vec2>();
	std::vector<glm::vec3> normals = std::vector<glm::vec3>();

	float* vertices = nullptr;
	int verticesSize = 0;
	int verticesCount = 0;

public:
	OBJLoader(const char* filepath);

	~OBJLoader() { delete[] vertices; };

private:
	glm::vec3 LoadVertexData(const std::string& data);
	std::vector<Vertex> LoadFace(const std::string& face);
	Vertex CreateVertex(const std::string& indicies);
	void CreateVertexArray(const std::vector<Vertex>& loadedVertices);

public:
	float* GetVertices() const { return vertices; }
	int GetVerticesSize() const { return verticesSize; }
	int GetVerticesCount() const { return verticesCount; }
};


// Indexed OBJLoader
class IOBJLoader {
	std::vector<glm::vec3> positions = std::vector<glm::vec3>();
	std::vector<int> indicies = std::vector<int>();

	float* vertices = nullptr;
	int verticesSize = 0;
	int verticesCount = 0;

public:
	IOBJLoader(const char* filepath);

	~IOBJLoader() { delete[] vertices; };

private:
	glm::vec3 LoadVertexData(const std::string& data);
	void LoadFace(const std::string& face);
	void CreateVertexArray();

public:
	float* GetVertices() const { return vertices; }
	int GetVerticesSize() const { return verticesSize; }
	int GetVerticesCount() const { return verticesCount; }

	inline const std::vector<int>& GetIndicies() const { return indicies; }
};

#endif // !OBJLOADER_H
