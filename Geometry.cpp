#include "Geometry.h"
#include <cmath> // Para sin, cos

std::vector<glm::vec3> generateCubePositions() {
    // Define os 8 cantos do cubo
    glm::vec3 p0 = {-1.0f, -1.0f,  1.0f}; // Frente inferior esquerda
    glm::vec3 p1 = { 1.0f, -1.0f,  1.0f}; // Frente inferior direita
    glm::vec3 p2 = { 1.0f,  1.0f,  1.0f}; // Frente superior direita
    glm::vec3 p3 = {-1.0f,  1.0f,  1.0f}; // Frente superior esquerda
    glm::vec3 p4 = {-1.0f, -1.0f, -1.0f}; // Trás inferior esquerda
    glm::vec3 p5 = { 1.0f, -1.0f, -1.0f}; // Trás inferior direita
    glm::vec3 p6 = { 1.0f,  1.0f, -1.0f}; // Trás superior direita
    glm::vec3 p7 = {-1.0f,  1.0f, -1.0f}; // Trás superior esquerda

    // Cria os vértices dos triângulos (36 vértices)
    std::vector<glm::vec3> vertices = {
        // Frente
        p0, p1, p2, p2, p3, p0,
        // Trás
        p4, p7, p6, p6, p5, p4,
        // Esquerda
        p4, p0, p3, p3, p7, p4,
        // Direita
        p1, p5, p6, p6, p2, p1,
        // Topo
        p3, p2, p6, p6, p7, p3,
        // Base
        p4, p5, p1, p1, p0, p4
    };
    return vertices;
}

std::vector<glm::vec3> generateCylinderPositions(int segments) {
    std::vector<glm::vec3> vertices;
    float radius = 1.0f;
    float height = 2.0f; // Altura total será 2 (-1 a 1 em Y)
    float halfHeight = height / 2.0f;

    float angleStep = 2.0f * glm::pi<float>() / segments;

    // Vértices da base e topo
    std::vector<glm::vec3> topVertices;
    std::vector<glm::vec3> bottomVertices;
    glm::vec3 topCenter(0.0f, halfHeight, 0.0f);
    glm::vec3 bottomCenter(0.0f, -halfHeight, 0.0f);

    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        topVertices.push_back(glm::vec3(x, halfHeight, z));
        bottomVertices.push_back(glm::vec3(x, -halfHeight, z));
    }

    // Triângulos do topo
    for (int i = 0; i < segments; ++i) {
        vertices.push_back(topCenter);
        vertices.push_back(topVertices[i]);
        vertices.push_back(topVertices[i + 1]);
    }

    // Triângulos da base
    for (int i = 0; i < segments; ++i) {
        vertices.push_back(bottomCenter);
        vertices.push_back(bottomVertices[i + 1]);
        vertices.push_back(bottomVertices[i]);
    }

    // Triângulos laterais (quads)
    for (int i = 0; i < segments; ++i) {
        // Triângulo 1
        vertices.push_back(bottomVertices[i]);
        vertices.push_back(topVertices[i]);
        vertices.push_back(topVertices[i + 1]);
        // Triângulo 2
        vertices.push_back(bottomVertices[i]);
        vertices.push_back(topVertices[i + 1]);
        vertices.push_back(bottomVertices[i + 1]);
    }

    return vertices;
}

void setupGeometry(const std::vector<glm::vec3>& vertices, GLuint& vao, GLuint& vbo, GLsizei& vertexCount) {
    if (vertices.empty()) return;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    // Atributo de Posição (layout = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Desvincula o VBO e o VAO para evitar modificações acidentais
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vertexCount = static_cast<GLsizei>(vertices.size());
}