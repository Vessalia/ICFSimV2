#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "VertexData.h"
#include "Shader.h"
#include "Material.h"

class Mesh
{
public:
	Mesh(const std::vector<VertexData<float>>& vertexData,
		const VertexData<unsigned int>& indexData, unsigned int renderHint);

	~Mesh();

	void draw(const Material* material) const;

private:
	unsigned int VAO;
	std::vector<unsigned int> VBOs;
	unsigned int EBO;

	VertexData<unsigned int> indices;

	size_t vertexCount;
};