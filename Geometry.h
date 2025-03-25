#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // Para pi()

// Gera os vértices de um cubo centrado na origem com lado 2
std::vector<glm::vec3> generateCubePositions();

// Gera os vértices de um cilindro centrado na origem, eixo Y, raio 1, altura 2
std::vector<glm::vec3> generateCylinderPositions(int segments = 32);

// Configura VAO e VBO para um conjunto de vértices
void setupGeometry(const std::vector<glm::vec3>& vertices, GLuint& vao, GLuint& vbo, GLsizei& vertexCount);

#endif // GEOMETRY_H