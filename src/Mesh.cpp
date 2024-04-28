#include "Mesh.h"
#include <glad/glad.h>

Mesh::Mesh(const std::vector<VertexData<float>>& vertexData,
    const VertexData<unsigned int>& indexData, unsigned int renderHint)
    : vertexCount(vertexData.size())
    , indices(indexData)
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    VBOs.resize(vertexData.size());

    glGenBuffers((GLsizei)vertexData.size(), &VBOs[0]);
    for (int i = 0; i < vertexData.size() && i < 16; i++)
    {
        int type = static_cast<int>(vertexData[i].type);

        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, vertexData[i].vertices.size() * sizeof(float), &vertexData[i].vertices[0], renderHint);

        glVertexAttribPointer(type, vertexData[i].size, GL_FLOAT, GL_FALSE, vertexData[i].stride * sizeof(float), (void*)(vertexData[i].offset * sizeof(float)));
        glEnableVertexAttribArray(type);
    }

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.vertices.size() * sizeof(unsigned int), &indexData.vertices[0], renderHint);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers((GLsizei)vertexCount, &VBOs[0]);
    glDeleteVertexArrays(1, &VAO);
}

void Mesh::draw(const Material* material) const
{
    material->use();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.vertices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}