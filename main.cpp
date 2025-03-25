#include <GL/glew.h> // GLEW primeiro
#include <GLFW/glfw3.h> // Depois GLFW
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Geometry.h"
#include "Constants.h"   // <-- Inclui as constantes globais
#include "Character.h"   // Inclui Character
#include "Mario.h"       // Inclui Mario

#include <iostream>
#include <vector>
#include <cmath> // Para atan2, sin, cos

// Protótipos de Funções
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height); // Comentado 'window' para silenciar aviso
void processInput(GLFWwindow *window, Character* character, float dt);
void drawShape(GLuint vao, GLsizei count, Shader& shader, glm::mat4 model, glm::vec3 color);

// Configurações
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Variáveis globais para acesso fácil pela classe Mario (não ideal, mas funciona)
// E para uso no main loop
GLuint cubeVAO, cubeVBO;
GLsizei cubeVertexCount;
GLuint cylinderVAO, cylinderVBO;
GLsizei cylinderVertexCount;

// DeltaTime
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Instância do Jogador (ponteiro para permitir polimorfismo futuro)
Character* player = nullptr; // Usaremos ponteiro da classe base

int main()
{
    // --- Inicialização GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // --- Criação da Janela GLFW ---
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mario Fan Game - Step 2 (Static Cam)", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- Inicialização GLEW ---
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }
    glGetError(); // Limpar erro inicial do GLEW

    // --- Configurações Globais OpenGL ---
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // --- Compilar e linkar shaders ---
    Shader ourShader("shaders/simple.vert", "shaders/simple.frag");

    // --- Configurar Geometria ---
    std::vector<glm::vec3> cubePositions = generateCubePositions();
    setupGeometry(cubePositions, cubeVAO, cubeVBO, cubeVertexCount);
    std::vector<glm::vec3> cylinderPositions = generateCylinderPositions(32);
    setupGeometry(cylinderPositions, cylinderVAO, cylinderVBO, cylinderVertexCount);

    // --- Criar Personagem ---
    player = new Mario(glm::vec3(0.0f, 0.0f, 0.0f)); // Cria o Mario na origem

    // --- Loop de Renderização ---
    while (!glfwWindowShouldClose(window))
    {
        // --- Calcular DeltaTime ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
         if (deltaTime > 0.1f) deltaTime = 0.1f; // Limitar deltaTime

        // --- Input ---
        processInput(window, player, deltaTime);

        // --- Atualizar Física ---
        if(player)
        {
            player->updatePhysics(deltaTime);
        }

        // --- Renderização ---
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Azul claro
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Ativar shader
        ourShader.use();

        // --- Matrizes de Transformação ---
        // Matriz de Projeção (Perspectiva) - Constante no loop se aspect ratio não muda
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // Matriz de Visualização (Câmera) - ESTÁTICA
        // Olhando para a origem (0,0,0) de uma posição fixa (ex: 0, 5, 15)
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 15.0f), // Posição da câmera fixa
                                     glm::vec3(0.0f, 1.0f, 0.0f), // Ponto para onde olha (um pouco acima do chão)
                                     glm::vec3(0.0f, 1.0f, 0.0f)); // Vetor 'up'
        ourShader.setMat4("view", view);

        // --- Desenhar Chão ---
        glm::mat4 floorModel = glm::mat4(1.0f);
        floorModel = glm::translate(floorModel, glm::vec3(0.0f, -0.05f, 0.0f));
        floorModel = glm::scale(floorModel, glm::vec3(15.0f, 0.1f, 15.0f));
        drawShape(cubeVAO, cubeVertexCount, ourShader, floorModel, glm::vec3(0.5f, 0.35f, 0.05f));

        // --- Desenhar Cano ---
        glm::mat4 pipeModel = glm::mat4(1.0f);
        pipeModel = glm::translate(pipeModel, glm::vec3(3.0f, 1.5f, -2.0f)); // Centro do cano
        pipeModel = glm::scale(pipeModel, glm::vec3(0.7f, 1.5f, 0.7f));
        drawShape(cylinderVAO, cylinderVertexCount, ourShader, pipeModel, glm::vec3(0.0f, 0.8f, 0.2f));

        // --- Desenhar Jogador ---
        if(player)
        {
            // A função draw do Mario/Character espera view e projection, mas elas já foram setadas no shader
            // Podemos passar ou não, mas a implementação atual de Mario::draw as re-seta.
            // Para evitar redundância, poderíamos modificar Mario::draw para *não* setar view/projection.
            // Por agora, deixaremos como está, mas cientes da redundância.
            player->draw(ourShader, view, projection);
        }

        // --- Trocar Buffers e Processar Eventos ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Limpeza ---
    delete player;
    player = nullptr;

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteBuffers(1, &cylinderVBO);
    glDeleteProgram(ourShader.ID);

    glfwTerminate();
    return 0;
}

// Função para desenhar uma forma genérica
void drawShape(GLuint vao, GLsizei count, Shader& shader, glm::mat4 model, glm::vec3 color) {
    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, count);
    glBindVertexArray(0);
}

// Processa input para o personagem
void processInput(GLFWwindow *window, Character* character, float dt) {
    if (!character) return;

    // Tenta converter para Mario* para acessar membros específicos
    Mario* mario = dynamic_cast<Mario*>(character);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // --- Rotação (Setas Esquerda/Direita) ---
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        character->rotationY -= PLAYER_ROTATION_SPEED * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        character->rotationY += PLAYER_ROTATION_SPEED * dt;
    }
    // Normaliza o ângulo de rotação (opcional, mas bom)
    // character->rotationY = fmod(character->rotationY, 360.0f);
    // if (character->rotationY < 0.0f) character->rotationY += 360.0f;


    // --- Inclinação da Cabeça (Setas Cima/Baixo) ---
    if (mario) { // Só funciona se a conversão para Mario* foi bem sucedida
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            mario->headTilt -= HEAD_TILT_SPEED * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            mario->headTilt += HEAD_TILT_SPEED * dt;
        }
        // Limita a inclinação da cabeça
        mario->headTilt = std::max(-MAX_HEAD_TILT, std::min(MAX_HEAD_TILT, mario->headTilt));
    }


    // --- Movimento (W/A/S/D - Relativo à Direção) ---
    glm::vec3 moveInput(0.0f); // Direção do input local (x=strafe, z=forward)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveInput.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveInput.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveInput.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveInput.x += 1.0f;

    bool isMovingInput = glm::length(moveInput) > 0.1f;

    if (isMovingInput) {
        moveInput = glm::normalize(moveInput);

        // Calcula vetores forward/right baseados na rotação ATUAL
        float angleRad = glm::radians(character->rotationY);
        glm::vec3 forward = glm::vec3(sin(angleRad), 0.0f, cos(angleRad));
        // Recalcula right explicitamente para garantir ortogonalidade
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        // Direção final do movimento no espaço do mundo
        glm::vec3 moveDir = forward * moveInput.z + right * moveInput.x;
         if (glm::length(moveDir) > 0.1f) { // Segurança extra na normalização
             moveDir = glm::normalize(moveDir);
         }


        // Aplica aceleração à velocidade
        character->velocity.x += moveDir.x * PLAYER_ACCELERATION * dt;
        character->velocity.z += moveDir.z * PLAYER_ACCELERATION * dt;

        // Define o estado de caminhada para animação (somente se for o Mario)
        if (mario) {
            mario->isWalking = true;
        }

        // REMOVIDO: A rotação agora é pelas setas
        // character->rotationY = glm::degrees(atan2(moveDir.x, moveDir.z));

    }
    // else { // Se não houver input de movimento
    //     if (mario) {
    //         mario->isWalking = false; // Parou de andar (será resetado em updatePhysics também)
    //     }
    // }

    // Limitar velocidade horizontal máxima (já existente, mantém)
    float maxHorizSpeed = character->speed;
    glm::vec2 horizVel(character->velocity.x, character->velocity.z);
    if (glm::length(horizVel) > maxHorizSpeed) {
        horizVel = glm::normalize(horizVel) * maxHorizSpeed;
        character->velocity.x = horizVel.x;
        character->velocity.z = horizVel.y;
    }


    // --- Pulo (Espaço) --- (Sem alterações na lógica de detecção)
    static bool spacePressedLastFrame = false;
    bool spacePressedThisFrame = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && character->onGround) {
        // Chama startJump diretamente se espaço pressionado E está no chão
        character->startJump();
        // Nota: Character::startJump ainda deve ter a checagem 'if (onGround)' por segurança
    }
    spacePressedLastFrame = spacePressedThisFrame;

    if (mario && isMovingInput && character->onGround) {
        mario->isWalking = true;
   }
}

// Callback de redimensionamento
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
}