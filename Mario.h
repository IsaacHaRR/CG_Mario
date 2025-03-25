#ifndef MARIO_H
#define MARIO_H

#include <GL/glew.h>
#include "Character.h"
#include <glm/gtc/constants.hpp> // Para pi

class Shader;

class Mario : public Character {
public:
    float headTilt;      // Inclinação da cabeça em graus
    bool isWalking;      // Estado de caminhada para animação
    float walkCycleTimer; // Timer para animação de caminhada

    Mario(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f));

    void draw(Shader& shader, glm::mat4 view, glm::mat4 projection) override;

    // Sobrescreve para atualizar estado de animação
    void updatePhysics(float deltaTime) override;

private:
    // Cores ... (sem alterações)
    glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 blue = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 skin = glm::vec3(1.0f, 0.8f, 0.6f);
    glm::vec3 brown = glm::vec3(0.4f, 0.2f, 0.0f);
    glm::vec3 yellow = glm::vec3(1.0f, 1.0f, 0.0f);

    // Função auxiliar de desenho (sem alterações na assinatura)
    void drawPart(GLuint vao, GLsizei count, Shader& shader, glm::mat4 model, glm::vec3 color);

    // Função auxiliar para calcular transformações animadas
    glm::mat4 getAnimatedPartTransform(float amplitude, float phaseOffset, const glm::vec3& rotationAxis, const glm::vec3& pivotOffset);
    glm::mat4 getJumpPoseTransform(bool isLeft, const glm::vec3& rotationAxis, const glm::vec3& pivotOffset);
};

#endif // MARIO_H