#include "Character.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h" // Inclui para usar Shader na assinatura de draw (embora seja virtual puro)

Character::Character(glm::vec3 startPos, float charHeight, float charSpeed, float charJump, float charGravity)
    : position(startPos),
      velocity(2.0f),
      rotationY(0.0f),
      onGround(false),
      isJumping(false),
      height(charHeight), // Usa a altura passada
      speed(charSpeed),
      jumpSpeed(charJump),
      gravity(charGravity) {}

void Character::updatePhysics(float deltaTime) {
    // Aplicar gravidade se não estiver no chão
    if (!onGround) {
        velocity.y += gravity * deltaTime;
    }

    // Atualizar posição baseada na velocidade
    position += velocity * deltaTime;

    // Colisão simples com o chão (y = 0)
    // Considera a "base" do personagem (position.y - height/2) mas para um chão em y=0,
    // basta verificar se position.y (centro) está abaixo de height/2.
    // Simplificação: vamos assumir que position.y representa a base por enquanto para facilitar.
    if (position.y < 0.0f) {
        position.y = 0.0f;
        if (velocity.y < 0.0f) { // Só para a queda se estiver caindo
             velocity.y = 0.0f;
        }
        onGround = true;
        isJumping = false; // Pousou
    } else {
        onGround = false;
    }

    // Aplicar damping horizontal (atrito simples)
    // Um valor menor de damping significa que desliza mais
    float dampingFactor = 0.1f; // Ajuste conforme necessário
    velocity.x *= (1.0f - dampingFactor);
    velocity.z *= (1.0f - dampingFactor);
    // Pequeno threshold para parar completamente
     if (glm::length(glm::vec2(velocity.x, velocity.z)) < 0.1f) {
         velocity.x = 0.0f;
         velocity.z = 0.0f;
     }
}

void Character::startJump() {
    if (onGround) {
        velocity.y = jumpSpeed;
        onGround = false;
        isJumping = true; // Pode ser útil para estados futuros
    }
}

glm::mat4 Character::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    // Escala pode ser adicionada aqui se o tamanho base mudar,
    // ou aplicada individualmente nas partes do corpo no draw() das classes filhas.
    return model;
}

// Implementação de draw é virtual pura, então não há corpo aqui.
// void Character::draw(Shader& shader, glm::mat4 view, glm::mat4 projection) {
//     // Implementação vazia ou erro se chamado diretamente
// }