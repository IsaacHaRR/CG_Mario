#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/glm.hpp>
#include <vector> // Para geometria no futuro, mas não essencial agora

// Forward declaration para evitar include circular se Character precisar de Shader
class Shader;

class Character {
public:
    // Estado
    glm::vec3 position;
    glm::vec3 velocity;
    float rotationY;
    bool onGround;
    bool isJumping; // Para diferenciar o início do pulo da queda

    // Atributos (podem ser sobrescritos por classes filhas)
    float speed;
    float jumpSpeed;
    float gravity;
    float height; // Altura aproximada para colisão com o chão

    Character(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f),
              float charHeight = 1.0f, // Altura padrão
              float charSpeed = 5.0f,
              float charJump = 8.0f,
              float charGravity = -18.0f);

    virtual ~Character() = default; // Destrutor virtual

    // Métodos
    virtual void updatePhysics(float deltaTime);
    virtual void startJump();
    virtual glm::mat4 getModelMatrix() const;

    // Método de desenho (virtual puro ou com implementação padrão vazia)
    // Requer view, projection e o shader para passar uniforms
    virtual void draw(Shader& shader, glm::mat4 view, glm::mat4 projection) = 0; // Virtual puro exige implementação nas classes filhas

protected:
    // Pode adicionar funções auxiliares aqui se necessário
};

#endif // CHARACTER_H