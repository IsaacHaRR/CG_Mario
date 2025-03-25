#include "Mario.h"
#include "Shader.h"
#include "Constants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath> // Para sin, cos
#include <algorithm> // Para std::lerp (interpolação)

// Funções/Variáveis externas (drawShape, VAOs/Counts)
extern void drawShape(GLuint vao, GLsizei count, Shader& shader, glm::mat4 model, glm::vec3 color);
extern GLuint cubeVAO;
extern GLsizei cubeVertexCount;

Mario::Mario(glm::vec3 startPos)
    : Character(startPos, 1.8f, PLAYER_SPEED, PLAYER_JUMP_SPEED, GRAVITY),
      headTilt(0.0f),
      isWalking(false),
      walkCycleTimer(0.0f)
{}

// Sobrescreve updatePhysics para gerenciar timer de caminhada
void Mario::updatePhysics(float deltaTime) {
    Character::updatePhysics(deltaTime); // Chama base

    // Atualiza timer de animação de caminhada com base na velocidade horizontal
    float horizontalSpeed = glm::length(glm::vec2(velocity.x, velocity.z));
    if (isWalking && onGround && horizontalSpeed > 0.1f) {
        // A velocidade da animação aumenta com a velocidade do personagem
        walkCycleTimer += WALK_ANIMATION_SPEED * (horizontalSpeed / PLAYER_SPEED) * deltaTime;
        walkCycleTimer = fmod(walkCycleTimer, 2.0f * glm::pi<float>()); // Mantém no ciclo 0-2PI
    } else if (onGround) {
         // Se está no chão mas parou, interpola suavemente para a pose parada (timer=0)
         walkCycleTimer = std::lerp(walkCycleTimer, 0.0f, 10.0f * deltaTime);
         if (abs(walkCycleTimer) < 0.1f) walkCycleTimer = 0.0f; // Encaixa no zero
    }
    // Se não estiver no chão, o timer não avança (mantém a pose do pulo)

    // Reset isWalking flag (será setada novamente em processInput)
    isWalking = false;
}


// Função auxiliar de desenho de partes (sem alterações)
void Mario::drawPart(GLuint vao, GLsizei count, Shader& shader, glm::mat4 model, glm::vec3 color) {
    drawShape(vao, count, shader, model, color);
}

// --- Funções Auxiliares de Animação Refinadas ---

// Calcula rotação para caminhada/corrida
glm::mat4 Mario::getWalkRotation(float amplitudeDegrees, float phaseOffset, const glm::vec3& rotationAxis, const glm::vec3& pivotOffset) {
    // Usar um seno mais acentuado ou outra função pode parecer menos robótico
    float angle = glm::radians(amplitudeDegrees) * sin(walkCycleTimer + phaseOffset);
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, pivotOffset);
    transform = glm::rotate(transform, angle, rotationAxis);
    transform = glm::translate(transform, -pivotOffset);
    return transform;
}

// Calcula rotação para pulo/queda baseado na velocidade Y
glm::mat4 Mario::getJumpRotation(bool isLeftLimb, const glm::vec3& rotationAxis, const glm::vec3& pivotOffset) {
    float angleDegrees = 0.0f;

    // Define ângulos base para subida e descida
    float riseAngle = isLeftLimb ? 45.0f : -20.0f; // Braço esq/Perna dir sobem mais
    float fallAngle = isLeftLimb ? -15.0f : 10.0f; // Braço esq/Perna dir descem/relaxam

    // Interpola o ângulo baseado na velocidade vertical para suavizar
    // Normaliza a velocidade Y para um range (-1 a 1, aproximadamente)
    float normalizedVy = glm::clamp(velocity.y / jumpSpeed, -1.0f, 1.0f);

    if (normalizedVy > 0.0f) { // Subindo
        angleDegrees = std::lerp(0.0f, riseAngle, normalizedVy); // Interpola de 0 até o ângulo de subida
    } else { // Caindo
        angleDegrees = std::lerp(0.0f, fallAngle, -normalizedVy); // Interpola de 0 até o ângulo de queda
    }

    // Aplica um balanço extra se estiver no pico (Vy perto de 0 mas não no chão)
     if (abs(velocity.y) < 1.0f && !onGround) {
          angleDegrees += sin( (glfwGetTime() * 4.0f) + (isLeftLimb ? glm::pi<float>() : 0.0f) ) * 5.0f; // Pequeno balanço no pico
     }


    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, pivotOffset);
    transform = glm::rotate(transform, glm::radians(angleDegrees), rotationAxis);
    transform = glm::translate(transform, -pivotOffset);
    return transform;
}


// --- Desenho do Mario Atualizado ---
void Mario::draw(Shader& shader, glm::mat4 view, glm::mat4 projection) {
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glm::mat4 baseModel = getModelMatrix();

    // --- Parâmetros de Animação ---
    float walkAmplitude = 45.0f; // Aumenta um pouco a amplitude
    glm::vec3 armRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 legRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);

    // Pivôs (podem precisar de ajuste fino)
    glm::vec3 leftArmPivot = glm::vec3(-0.4f, 1.3f, 0.0f);
    glm::vec3 rightArmPivot = glm::vec3(0.4f, 1.3f, 0.0f);
    glm::vec3 leftLegPivot = glm::vec3(-0.15f, 0.8f, 0.0f);
    glm::vec3 rightLegPivot = glm::vec3(0.15f, 0.8f, 0.0f);

    // Calcular matrizes de animação para membros
    glm::mat4 leftArmAnim = glm::mat4(1.0f);
    glm::mat4 rightArmAnim = glm::mat4(1.0f);
    glm::mat4 leftLegAnim = glm::mat4(1.0f);
    glm::mat4 rightLegAnim = glm::mat4(1.0f);

    if (!onGround) { // Animação de Pulo/Queda
        leftArmAnim = getJumpRotation(true, armRotationAxis, leftArmPivot);
        rightArmAnim = getJumpRotation(false, armRotationAxis, rightArmPivot);
        leftLegAnim = getJumpRotation(false, legRotationAxis, leftLegPivot); // Perna esquerda (segue braço direito)
        rightLegAnim = getJumpRotation(true, legRotationAxis, rightLegPivot); // Perna direita (segue braço esquerdo)
    } else { // Animação de Caminhada (se walkCycleTimer > 0)
        leftArmAnim = getWalkRotation(walkAmplitude, glm::pi<float>(), armRotationAxis, leftArmPivot);
        rightArmAnim = getWalkRotation(walkAmplitude, 0.0f, armRotationAxis, rightArmPivot);
        leftLegAnim = getWalkRotation(walkAmplitude, 0.0f, legRotationAxis, leftLegPivot);
        rightLegAnim = getWalkRotation(walkAmplitude, glm::pi<float>(), legRotationAxis, rightLegPivot);
    }

    // --- Desenhar Cabeça (Nariz/Bigode Maiores) ---
    glm::mat4 headTransform = glm::translate(baseModel, glm::vec3(0.0f, 1.6f, 0.0f));
    headTransform = glm::rotate(headTransform, glm::radians(headTilt), glm::vec3(1.0f, 0.0f, 0.0f));

    // Cabeça Base (Pele)
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(headTransform, glm::vec3(0.3f)), skin);
    // Nariz (Pele) - Maior e mais à frente
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(headTransform, glm::vec3(0.0f, -0.02f, 0.28f)), glm::vec3(0.16f, 0.18f, 0.22f)), skin);
    // Bigode (Marrom) - Mais largo e espesso
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(headTransform, glm::vec3(0.0f, -0.14f, 0.26f)), glm::vec3(0.4f, 0.1f, 0.12f)), brown);
    // Boné (Vermelho) - Sem alterações
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(headTransform, glm::vec3(0.0f, 0.2f, 0.0f)), glm::vec3(0.35f, 0.15f, 0.35f)), red);
    // Aba do boné - Sem alterações
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(headTransform, glm::vec3(0.0f, 0.15f, 0.22f)), glm::vec3(0.35f, 0.05f, 0.15f)), red);

    // --- Desenhar Corpo (com leve Bob) ---
    float torsoBob = onGround ? (0.03f * sin(walkCycleTimer * 2.0f)) : 0.0f; // Bob só no chão
    glm::vec3 torsoOffset = glm::vec3(0.0f, 0.9f + torsoBob, 0.0f);
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(baseModel, torsoOffset), glm::vec3(0.4f, 0.5f, 0.2f)), blue);

    // --- Desenhar Pernas e Braços (Animados) ---
    // Aplica a matriz de animação à matriz base ANTES de transladar/escalar a parte
    glm::mat4 leftLegModel = baseModel * leftLegAnim;
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(leftLegModel, glm::vec3(-0.15f, 0.4f, 0.0f)), glm::vec3(0.15f, 0.4f, 0.15f)), blue);
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(leftLegModel, glm::vec3(-0.15f, 0.05f, 0.05f)), glm::vec3(0.15f, 0.1f, 0.2f)), brown); // Sapato Esquerdo

    glm::mat4 rightLegModel = baseModel * rightLegAnim;
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(rightLegModel, glm::vec3(0.15f, 0.4f, 0.0f)), glm::vec3(0.15f, 0.4f, 0.15f)), blue);
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(rightLegModel, glm::vec3(0.15f, 0.05f, 0.05f)), glm::vec3(0.15f, 0.1f, 0.2f)), brown); // Sapato Direito

    glm::mat4 leftArmModel = baseModel * leftArmAnim;
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(leftArmModel, glm::vec3(-0.5f, 0.9f, 0.0f)), glm::vec3(0.1f, 0.4f, 0.15f)), red);
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(leftArmModel, glm::vec3(-0.5f, 0.55f, 0.0f)), glm::vec3(0.12f, 0.12f, 0.12f)), skin); // Mão Esquerda

    glm::mat4 rightArmModel = baseModel * rightArmAnim;
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(rightArmModel, glm::vec3(0.5f, 0.9f, 0.0f)), glm::vec3(0.1f, 0.4f, 0.15f)), red);
    drawPart(cubeVAO, cubeVertexCount, shader, glm::scale(glm::translate(rightArmModel, glm::vec3(0.5f, 0.55f, 0.0f)), glm::vec3(0.12f, 0.12f, 0.12f)), skin); // Mão Direita
}