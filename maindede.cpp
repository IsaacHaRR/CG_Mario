#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm> // Para std::min/max
#include <random>    // For better random numbers

// --- Constantes e Configurações ---
const unsigned int SCR_WIDTH = 1024; // Wider screen for more space
const unsigned int SCR_HEIGHT = 768;
const float GROUND_SIZE = 60.0f; // Make ground larger for wandering
const float WANDER_RADIUS = GROUND_SIZE / 2.0f - 5.0f; // Max distance from center for NPCs
const float FLYING_MIN_Y = 2.0f;
const float FLYING_MAX_Y = 8.0f;

// --- Cores Adventure Time ---
const glm::vec3 COLOR_FINN_SKIN(1.0f, 0.85f, 0.7f);
const glm::vec3 COLOR_FINN_SHIRT(0.2f, 0.7f, 0.9f);
const glm::vec3 COLOR_FINN_PANTS(0.0f, 0.2f, 0.5f);
const glm::vec3 COLOR_FINN_HAT(1.0f, 1.0f, 1.0f);
const glm::vec3 COLOR_FINN_BACKPACK(0.1f, 0.6f, 0.1f);
const glm::vec3 COLOR_BLACK(0.0f, 0.0f, 0.0f);
const glm::vec3 COLOR_WHITE(1.0f, 1.0f, 1.0f);
const glm::vec3 COLOR_JAKE_BODY(1.0f, 0.8f, 0.0f);
const glm::vec3 COLOR_SKY_BLUE(0.5f, 0.8f, 1.0f);
const glm::vec3 COLOR_GRASS_GREEN(0.3f, 0.7f, 0.3f);
const glm::vec3 COLOR_SWORD_GREY(0.7f, 0.7f, 0.7f);
// New Character Colors
const glm::vec3 COLOR_BMO_BODY(0.4f, 0.8f, 0.75f); // Tealish
const glm::vec3 COLOR_BMO_SCREEN(0.1f, 0.2f, 0.15f);
const glm::vec3 COLOR_BMO_BUTTON_RED(1.0f, 0.2f, 0.2f);
const glm::vec3 COLOR_BMO_BUTTON_BLUE(0.2f, 0.3f, 1.0f);
const glm::vec3 COLOR_BMO_BUTTON_YELLOW(1.0f, 0.9f, 0.2f);
const glm::vec3 COLOR_ICE_KING_BODY(0.6f, 0.8f, 1.0f); // Light Blue
const glm::vec3 COLOR_ICE_KING_BEARD(0.9f, 0.95f, 1.0f); // Off-white
const glm::vec3 COLOR_ICE_KING_CROWN(1.0f, 0.9f, 0.0f); // Yellow
const glm::vec3 COLOR_ICE_KING_GEM(1.0f, 0.1f, 0.1f);   // Red
const glm::vec3 COLOR_PB_SKIN(1.0f, 0.75f, 0.85f); // Pinkish skin
const glm::vec3 COLOR_PB_HAIR(1.0f, 0.4f, 0.7f);   // Bright Pink
const glm::vec3 COLOR_PB_DRESS(0.9f, 0.5f, 0.75f);  // Slightly darker pink
const glm::vec3 COLOR_PB_CROWN(1.0f, 0.9f, 0.0f);   // Yellow
const glm::vec3 COLOR_PB_GEM(0.2f, 0.7f, 0.8f);   // Blue gem
const glm::vec3 COLOR_MARCELINE_SKIN(0.8f, 0.85f, 0.9f); // Greyish blue
const glm::vec3 COLOR_MARCELINE_HAIR(0.15f, 0.15f, 0.2f); // Very Dark Grey
const glm::vec3 COLOR_MARCELINE_SHIRT(0.7f, 0.1f, 0.1f);  // Dark Red
const glm::vec3 COLOR_MARCELINE_PANTS(0.1f, 0.1f, 0.3f);  // Dark Blue
const glm::vec3 COLOR_MARCELINE_BASS(0.9f, 0.1f, 0.1f); // Red Bass


// --- Random Number Generator ---
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> distrib(-1.0f, 1.0f); // For directions
std::uniform_real_distribution<float> distrib01(0.0f, 1.0f); // For probabilities/intervals
std::uniform_real_distribution<float> distribFlyY(FLYING_MIN_Y, FLYING_MAX_Y);
std::uniform_real_distribution<float> distribWander(-WANDER_RADIUS, WANDER_RADIUS);


// --- Estrutura de Vértice ---
// struct Vertex { // Not used as color is uniform
//     glm::vec3 Position;
// };

// --- Código dos Shaders (Inalterado) ---
const char* vertexShaderSource = R"(#version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 model; uniform mat4 view; uniform mat4 projection;
    void main() { gl_Position = projection * view * model * vec4(aPos, 1.0); }
)";
const char* fragmentShaderSource = R"(#version 330 core
    out vec4 FinalColor; uniform vec3 objectColor;
    void main() { FinalColor = vec4(objectColor, 1.0f); }
)";

// --- Variáveis Globais para OpenGL (Inalterado) ---
GLuint shaderProgram;
GLuint cubeVAO, cubeVBO;
GLuint pyramidVAO, pyramidVBO;
GLuint coneVAO, coneVBO;
GLsizei cubeVertexCount;
GLsizei pyramidVertexCount;
GLsizei coneVertexCount;
bool wireframeMode = false;
bool zeroKeyPressedLastFrame = false;

// --- Forward Declarations of Functions ---
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
void setupGeometry(GLuint& vao, GLuint& vbo, const std::vector<glm::vec3>& positions, GLsizei& vertexCount);
std::vector<glm::vec3> generateCubePositions();
std::vector<glm::vec3> generatePyramidPositions();
std::vector<glm::vec3> generateConePositions(int slices = 16);
void drawShape(GLuint vao, GLsizei vertexCount, glm::mat4 model, const glm::vec3& color);

// --- Forward Declarations of Classes ---
class Character;
class Finn;
class Jake;
class BMO;
class IceKing;
class PrincessBubblegum;
class Marceline;

// --- Classe base Character ---
class Character {
public:
    glm::vec3 position;
    float rotation; // Y-axis rotation (radians) for facing direction
    float headInclination; // X-axis rotation for looking up/down (radians)
    float speed;
    bool isJumping;
    float currentVerticalSpeed;
    float gravity;
    float groundHeight;
    float initialJumpSpeed; // Added member to store this

    float legSwingAngle;
    float armSwingAngle;
    bool moving; // Set by movement input or NPC logic

    // NPC Specific Wander Behavior
    glm::vec3 targetPosition;
    float timeSinceLastDecision;
    float decisionInterval;
    bool isNPC = false; // Flag to distinguish NPCs
    bool isUnderPlayerControl = false; // Flag set in main loop

    Character(glm::vec3 pos, float rot, float inc, float spd, float jumpInitialSpd, float g, float grndHeight = 0.0f, bool npc = false); // Declaration only

    virtual ~Character() {}

    // Update method declaration
    virtual void update(float deltaTime);

    // NPC Wander Logic declarations
    virtual void chooseNewTarget();
    void updateNPCWander(float deltaTime); // Implementation moved later

    // Player Character Methods declarations
    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void rotateLeft(float deltaTime);
    void rotateRight(float deltaTime);
    void startJump(float jumpInitialSpeed); // Implementation moved later
    void updateJump(float deltaTime); // Implementation moved later
    void updateLimbSwing(float deltaTime);
};

// --- Classes Jogáveis (Finn e Jake) ---
const float JAKE_BASE_LEG_LENGTH = 0.5f;

class Finn : public Character {
public:
    bool isAttacking;
    float attackStartTime;

    // Constructor for Finn
    Finn(glm::vec3 pos) : Character(pos, 0.0f, 0.0f, 7.0f, 10.0f, 25.0f, 0.0f, false), // Base stats for Finn, isNPC=false
                          isAttacking(false), attackStartTime(0.0f) {}

    // Finn-specific methods declaration
    void startAttack();
    void update(float deltaTime) override; // Override necessary
};

class Jake : public Character {
public:
    float legStretch;
    float sizeMultiplier;

    Jake(glm::vec3 pos) : Character(pos, 0.0f, 0.0f, 6.0f, 9.0f, 28.0f, 0.0f, false), // Base stats for Jake, isNPC=false
                          legStretch(1.0f), sizeMultiplier(1.0f) {}

    float getStretchHeightOffset() const;
    float getEffectiveGroundHeight() const;
    void update(float deltaTime) override; // Override necessary
};

// --- Classes NPC ---

class BMO : public Character {
public:
    BMO(glm::vec3 pos) : Character(pos, 0.0f, 0.0f, 2.5f, 0.0f, 9.8f, 0.0f, true) {} // Slower speed, NPC=true

    void chooseNewTarget() override;
    void update(float deltaTime) override;
};

class PrincessBubblegum : public Character {
public:
    PrincessBubblegum(glm::vec3 pos) : Character(pos, 0.0f, 0.0f, 3.0f, 0.0f, 9.8f, 0.0f, true) {} // NPC=true

    void chooseNewTarget() override;
    void update(float deltaTime) override;
};

class IceKing : public Character {
public:
    IceKing(glm::vec3 pos) : Character(pos, 0.0f, 0.0f, 3.5f, 0.0f, 0.0f, 0.0f, true) { // NPC=true, No gravity needed
        position.y = distribFlyY(gen); // Start flying
        chooseNewTarget(); // Set initial flying target
    }

    void chooseNewTarget() override;
    // Inherits Character::update, which calls updateNPCWander if not player-controlled
};

class Marceline : public Character {
public:
    Marceline(glm::vec3 pos) : Character(pos, 0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, true) { // NPC=true, Faster flyer, No gravity
        position.y = distribFlyY(gen); // Start flying
        chooseNewTarget(); // Set initial flying target
    }
     void chooseNewTarget() override;
     // Inherits Character::update, which calls updateNPCWander if not player-controlled
};


// --- Funções de Desenho dos Personagens (Declarations) ---
void drawFinn(Finn* finn, const glm::mat4& view, const glm::mat4& projection);
void drawJake(Jake* jake, const glm::mat4& view, const glm::mat4& projection);
void drawBMO(BMO* bmo, const glm::mat4& view, const glm::mat4& projection);
void drawIceKing(IceKing* ik, const glm::mat4& view, const glm::mat4& projection);
void drawPB(PrincessBubblegum* pb, const glm::mat4& view, const glm::mat4& projection);
void drawMarceline(Marceline* marcy, const glm::mat4& view, const glm::mat4& projection);


// --- Implementações das classes NPC ---

void BMO::chooseNewTarget() {
    targetPosition = glm::vec3(distribWander(gen), groundHeight, distribWander(gen));
    timeSinceLastDecision = 0.0f;
    decisionInterval = 4.0f + distrib01(gen) * 6.0f; // 4-10s
}

void BMO::update(float deltaTime) {
    Character::update(deltaTime); // Calls base update (handles wander if not controlled)
    // Ensure BMO stays exactly on the ground if not jumping
    if (!isJumping) {
        position.y = groundHeight;
    }
}

void PrincessBubblegum::chooseNewTarget() {
    targetPosition = glm::vec3(distribWander(gen), groundHeight, distribWander(gen));
    timeSinceLastDecision = 0.0f;
    decisionInterval = 5.0f + distrib01(gen) * 5.0f; // 5-10s
}

void PrincessBubblegum::update(float deltaTime) {
    Character::update(deltaTime); // Calls base update (handles wander if not controlled)
    // Ensure PB stays exactly on the ground if not jumping
     if (!isJumping) {
        position.y = groundHeight;
    }
}

void IceKing::chooseNewTarget() {
    targetPosition = glm::vec3(distribWander(gen), distribFlyY(gen), distribWander(gen)); // Target includes random Y
    timeSinceLastDecision = 0.0f;
    decisionInterval = 6.0f + distrib01(gen) * 6.0f; // 6-12s
}
// IceKing uses base Character::update

void Marceline::chooseNewTarget() {
    targetPosition = glm::vec3(distribWander(gen), distribFlyY(gen), distribWander(gen)); // Target includes random Y
    timeSinceLastDecision = 0.0f;
    decisionInterval = 4.0f + distrib01(gen) * 4.0f; // 4-8s (more erratic?)
}
// Marceline uses base Character::update


// --- Implementações Character / Finn / Jake (DEPOIS de todas as class declarations) ---

// Character Constructor Definition
Character::Character(glm::vec3 pos, float rot, float inc, float spd, float jumpInitialSpd, float g, float grndHeight, bool npc) :
    position(pos), rotation(rot), headInclination(inc), speed(spd), isJumping(false),
    currentVerticalSpeed(0.0f), gravity(g), groundHeight(grndHeight), initialJumpSpeed(jumpInitialSpd), // Initialize initialJumpSpeed
    legSwingAngle(0.0f), armSwingAngle(0.0f), moving(false),
    targetPosition(pos), timeSinceLastDecision(0.0f), decisionInterval(5.0f + distrib01(gen) * 5.0f),
    isNPC(npc), isUnderPlayerControl(false) // Initialize new flag
{
    // Don't call chooseNewTarget here, let derived NPC constructors do it
}

// Character update DEFINITION
void Character::update(float deltaTime) {
    // Always apply gravity/jump physics
    updateJump(deltaTime);

    // Update limb swing (mainly for Finn/Jake appearance)
    updateLimbSwing(deltaTime);

    // Handle NPC wandering ONLY if it's an NPC and NOT under player control
    if (isNPC && !isUnderPlayerControl) {
        updateNPCWander(deltaTime);
    }

    // Reset moving flag IF NOT under player control (player control sets it via input)
    if (!isUnderPlayerControl) {
        moving = false; // NPCs set 'moving' in wander logic if they move
    } else {
         moving = false; // Reset player moving flag each frame, set true on move input
    }
}

// ***** ADD THIS DEFINITION *****
void Character::chooseNewTarget() {
    // Default implementation for the base class or non-overriding derived classes.
    // Choose a random target on the ground.
    targetPosition = glm::vec3(distribWander(gen), groundHeight, distribWander(gen));
    timeSinceLastDecision = 0.0f;
    // Set a default decision interval
    decisionInterval = 5.0f + distrib01(gen) * 5.0f; // Random interval 5-10s
}
// ***** END OF ADDED DEFINITION *****

// Character updateNPCWander DEFINITION (uses dynamic_cast, needs derived class definitions)
void Character::updateNPCWander(float deltaTime) {
    timeSinceLastDecision += deltaTime;
    // Check distance OR time interval to pick new target
    if (timeSinceLastDecision > decisionInterval || glm::distance(position, targetPosition) < 1.0f) {
        chooseNewTarget(); // Calls the VIRTUAL function (derived implementation if exists)
    }

    glm::vec3 direction = targetPosition - position;
    float distanceToTarget = glm::length(direction);

    // Only move and rotate if not already at the target
    if (distanceToTarget > 0.1f) {
        glm::vec3 moveDir = glm::normalize(direction);

        // Rotate to face the target direction
        rotation = atan2(moveDir.x, moveDir.z);

        // Move towards target
        position += moveDir * speed * deltaTime;
        moving = true; // Indicate movement

        // Ensure walking NPCs don't accidentally change Y due to float inaccuracy while moving
        // Check if 'this' is NOT a flyer using dynamic_cast
         if (!dynamic_cast<IceKing*>(this) && !dynamic_cast<Marceline*>(this)){
             // Ensure Y stays at ground height ONLY IF NOT JUMPING
             // (This check might be redundant if jump logic handles ground snapping well)
             if (!isJumping) {
                position.y = groundHeight;
             }
         }

    } else {
        moving = false; // Reached target
        // Snap walkers to ground height precisely when stopped
        // Check if 'this' is NOT a flyer
        if (!dynamic_cast<IceKing*>(this) && !dynamic_cast<Marceline*>(this)){
            position.y = groundHeight;
            isJumping = false; // Ensure grounded state if snapped
            currentVerticalSpeed = 0.0f;
        }
    }
}


// Character startJump DEFINITION (uses dynamic_cast, needs Jake definition)
void Character::startJump(float jumpInitialSpeed) {
    float effectiveGround = groundHeight;
    // Check if 'this' is actually a Jake object
    if (const Jake* j = dynamic_cast<const Jake*>(this)) {
        effectiveGround = j->getEffectiveGroundHeight();
    }
    // Allow jump only if not already jumping and close to the effective ground
    // Removed !isNPC check - allow controlled NPCs to jump
    if (!isJumping && abs(position.y - effectiveGround) < 0.15f) { // Slightly larger tolerance
        isJumping = true;
        currentVerticalSpeed = jumpInitialSpeed; // Use the passed-in value
    }
}

// Character updateJump DEFINITION (uses dynamic_cast, needs Jake definition)
void Character::updateJump(float deltaTime) {
    // Apply gravity if airborne or moving upwards
    // Flyers (IceKing, Marceline) have gravity 0, so this won't affect them negatively
     if (position.y > groundHeight || currentVerticalSpeed > 0 || isJumping) { // Keep applying gravity until landed
        currentVerticalSpeed -= gravity * deltaTime;
     }

    // Update position based on vertical speed
    position.y += currentVerticalSpeed * deltaTime;

    // Determine the target ground height (might be higher for stretched Jake)
    float targetGround = groundHeight;
    if (Jake* j = dynamic_cast<Jake*>(this)) {
        targetGround = j->getEffectiveGroundHeight();
    }

    // Check for landing (only if moving downwards or at ground level)
    if (position.y <= targetGround && currentVerticalSpeed <= 0) {
        position.y = targetGround; // Snap to ground
        isJumping = false;         // Stop jumping state
        currentVerticalSpeed = 0.0f; // Reset vertical speed
    }
}

// Character updateLimbSwing DEFINITION
void Character::updateLimbSwing(float deltaTime) {
    // Only applies animation to Finn/Jake appearance
    if (dynamic_cast<Finn*>(this) || dynamic_cast<Jake*>(this)) {
        const float swingSpeed = 6.0f;
        const float maxSwingAngle = glm::radians(40.0f);
        const float armMultiplier = 1.2f;

        if (moving) { // 'moving' is set by input processing or NPC wander
            float time = (float)glfwGetTime(); // Use global time for consistent swing
            legSwingAngle = sin(time * swingSpeed) * maxSwingAngle;
            armSwingAngle = -sin(time * swingSpeed) * maxSwingAngle * armMultiplier; // Arms swing opposite
        } else {
            // Dampen swing when stopped
            legSwingAngle *= pow(0.1f, deltaTime);
            armSwingAngle *= pow(0.1f, deltaTime);
            if (abs(legSwingAngle) < 0.01f) legSwingAngle = 0.0f;
            if (abs(armSwingAngle) < 0.01f) armSwingAngle = 0.0f;
        }
    } else {
        // Ensure non-Finn/Jake have zero swing
        legSwingAngle = 0.0f;
        armSwingAngle = 0.0f;
    }
}

// --- Movement Methods (Apply directly to character, no NPC check needed here) ---
void Character::moveForward(float deltaTime) {
    position.x += speed * deltaTime * sin(rotation);
    position.z += speed * deltaTime * cos(rotation);
    moving = true;
}

void Character::moveBackward(float deltaTime) {
    position.x -= speed * deltaTime * sin(rotation);
    position.z -= speed * deltaTime * cos(rotation);
    moving = true;
}

void Character::rotateLeft(float deltaTime) {
    rotation += 2.0f * deltaTime;
    rotation = fmod(rotation, 2.0f * glm::pi<float>());
    if (rotation < 0.0f) rotation += 2.0f * glm::pi<float>();
}

void Character::rotateRight(float deltaTime) {
    rotation -= 2.0f * deltaTime;
    rotation = fmod(rotation, 2.0f * glm::pi<float>());
    if (rotation < 0.0f) rotation += 2.0f * glm::pi<float>();
}


// --- Finn Implementations ---
void Finn::startAttack() {
    if (!isAttacking) {
        isAttacking = true;
        attackStartTime = (float)glfwGetTime();
    }
}

void Finn::update(float deltaTime) {
    Character::update(deltaTime); // Call base class update
    // Update attack state specific to Finn
    if (isAttacking && (glfwGetTime() - attackStartTime > 0.3f)) { // Attack duration
        isAttacking = false;
    }
}

// --- Jake Implementations ---
float Jake::getStretchHeightOffset() const {
    if (legStretch > 1.0f) {
        return JAKE_BASE_LEG_LENGTH * (legStretch - 1.0f);
    }
    return 0.0f;
}

float Jake::getEffectiveGroundHeight() const {
    return groundHeight + getStretchHeightOffset();
}

void Jake::update(float deltaTime) {
    Character::update(deltaTime); // Call base class update FIRST

    // --- Update Jake-specific properties ---
    if (legStretch > 1.0f) {
        legStretch -= 3.0f * deltaTime;
        legStretch = std::max(1.0f, legStretch);
    }
    if(sizeMultiplier > 1.0f){
        sizeMultiplier -= 2.0f * deltaTime;
        sizeMultiplier = std::max(1.0f, sizeMultiplier);
    }

    // --- Adjust Position based on Stretch ---
    // Ensure Jake is at the correct height IF he's not jumping
    float targetGround = getEffectiveGroundHeight();
    if (!isJumping && abs(position.y - targetGround) > 0.01f) {
        position.y = targetGround;
        if (currentVerticalSpeed > 0) currentVerticalSpeed = 0.0f; // Kill upward speed if snapped down
    }
    // Landing is handled correctly by Character::updateJump using targetGround.
}

// --- Implementações das Funções de Desenho (Colocadas aqui, após classes) ---

void drawFinn(Finn* finn, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 finnModel = glm::mat4(1.0f);
    finnModel = glm::translate(finnModel, finn->position);
    finnModel = glm::rotate(finnModel, finn->rotation, glm::vec3(0.0f, 1.0f, 0.0f));

    // Torso (Shirt)
    glm::mat4 torsoModel = glm::translate(finnModel, glm::vec3(0.0f, 0.6f, 0.0f));
    torsoModel = glm::scale(torsoModel, glm::vec3(0.5f, 0.7f, 0.3f));
    drawShape(cubeVAO, cubeVertexCount, torsoModel, COLOR_FINN_SHIRT);

    // Head
    glm::mat4 headModel = glm::translate(finnModel, glm::vec3(0.0f, 1.2f, 0.0f));
    headModel = glm::rotate(headModel, finn->headInclination, glm::vec3(1.0f, 0.0f, 0.0f));
    headModel = glm::scale(headModel, glm::vec3(0.4f, 0.4f, 0.4f));
    drawShape(cubeVAO, cubeVertexCount, headModel, COLOR_FINN_SKIN);

    // Hat Base (on head)
    glm::mat4 hatBaseModel = glm::translate(headModel, glm::vec3(0.0f, 0.1f, 0.0f)); // Slight offset from head center
    hatBaseModel = glm::scale(hatBaseModel, glm::vec3(1.1f, 1.0f, 1.1f)); // Slightly larger than head scale
    drawShape(cubeVAO, cubeVertexCount, hatBaseModel, COLOR_FINN_HAT);

    // Hat Ears (relative to hat base)
    glm::mat4 earLModel = glm::translate(hatBaseModel, glm::vec3(-0.4f, 0.6f, 0.0f));
    earLModel = glm::scale(earLModel, glm::vec3(0.2f, 0.4f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, earLModel, COLOR_FINN_HAT);
    glm::mat4 earRModel = glm::translate(hatBaseModel, glm::vec3(0.4f, 0.6f, 0.0f));
    earRModel = glm::scale(earRModel, glm::vec3(0.2f, 0.4f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, earRModel, COLOR_FINN_HAT);


    // Legs (Pants) - Apply swing
    glm::mat4 legLModel = glm::translate(finnModel, glm::vec3(-0.15f, 0.0f, 0.0f)); // Initial pos
    legLModel = glm::translate(legLModel, glm::vec3(0.0f, 0.15f, 0.0f)); // Move pivot up
    legLModel = glm::rotate(legLModel, finn->legSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate
    legLModel = glm::translate(legLModel, glm::vec3(0.0f, -0.15f, 0.0f)); // Move back down
    legLModel = glm::scale(legLModel, glm::vec3(0.2f, 0.5f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, legLModel, COLOR_FINN_PANTS);

    glm::mat4 legRModel = glm::translate(finnModel, glm::vec3(0.15f, 0.0f, 0.0f)); // Initial pos
    legRModel = glm::translate(legRModel, glm::vec3(0.0f, 0.15f, 0.0f)); // Pivot
    legRModel = glm::rotate(legRModel, -finn->legSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate opposite
    legRModel = glm::translate(legRModel, glm::vec3(0.0f, -0.15f, 0.0f)); // Move back down
    legRModel = glm::scale(legRModel, glm::vec3(0.2f, 0.5f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, legRModel, COLOR_FINN_PANTS);

    // Arms (Shirt color) - Apply swing
    glm::mat4 armLModel = glm::translate(finnModel, glm::vec3(-0.35f, 0.9f, 0.0f)); // Initial pos at shoulder
    armLModel = glm::rotate(armLModel, finn->armSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate from shoulder
    armLModel = glm::translate(armLModel, glm::vec3(0.0f, -0.3f, 0.0f)); // Move down to arm center
    armLModel = glm::scale(armLModel, glm::vec3(0.15f, 0.6f, 0.15f));
    drawShape(cubeVAO, cubeVertexCount, armLModel, COLOR_FINN_SHIRT); // Shirt sleeve

    glm::mat4 armRModel = glm::translate(finnModel, glm::vec3(0.35f, 0.9f, 0.0f)); // Shoulder
    // Attack animation for right arm
    float rightArmAngle = -finn->armSwingAngle; // Default opposite swing
    if(finn->isAttacking){
         float attackProgress = (float)glfwGetTime() - finn->attackStartTime;
         rightArmAngle = glm::radians(-90.0f + sin(attackProgress / 0.3f * glm::pi<float>()) * 90.0f); // Simple swing forward
    }
    armRModel = glm::rotate(armRModel, rightArmAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate
    armRModel = glm::translate(armRModel, glm::vec3(0.0f, -0.3f, 0.0f)); // Center
    armRModel = glm::scale(armRModel, glm::vec3(0.15f, 0.6f, 0.15f));
    drawShape(cubeVAO, cubeVertexCount, armRModel, COLOR_FINN_SHIRT);

    // Backpack
    glm::mat4 packModel = glm::translate(finnModel, glm::vec3(0.0f, 0.6f, -0.2f));
    packModel = glm::scale(packModel, glm::vec3(0.4f, 0.5f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, packModel, COLOR_FINN_BACKPACK);

     // Sword (only when attacking)
    if (finn->isAttacking) {
        // Attach sword to the rotated right arm model
        glm::mat4 swordModel = armRModel; // Start with the final arm model matrix
        swordModel = glm::translate(swordModel, glm::vec3(0.0f, -0.7f, 0.1f)); // Position relative to arm center (down and slightly forward)
        swordModel = glm::scale(swordModel, glm::vec3(0.1f / 0.15f, 1.0f / 0.6f, 0.5f / 0.15f)); // Counter-act arm scale, make blade long
        swordModel = glm::scale(swordModel, glm::vec3(0.1f, 1.0f, 0.05f)); // Actual sword dimensions
        drawShape(cubeVAO, cubeVertexCount, swordModel, COLOR_SWORD_GREY);
    }
}

void drawJake(Jake* jake, const glm::mat4& view, const glm::mat4& projection) {
    // Base model incorporates position, rotation, and overall size multiplier
    glm::mat4 jakeModelBase = glm::mat4(1.0f);
    // Adjust base position by stretch offset so feet stay grounded when stretching
    glm::vec3 basePos = jake->position - glm::vec3(0.0f, jake->getStretchHeightOffset(), 0.0f);
    jakeModelBase = glm::translate(jakeModelBase, basePos);
    jakeModelBase = glm::rotate(jakeModelBase, jake->rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    jakeModelBase = glm::scale(jakeModelBase, glm::vec3(jake->sizeMultiplier)); // Apply overall size


    // Body (main part) - Scale Y by legStretch
    glm::mat4 bodyModel = jakeModelBase;
    float bodyCenterY = 0.5f * JAKE_BASE_LEG_LENGTH * jake->legStretch + 0.35f; // Center calculation based on stretched legs
    bodyModel = glm::translate(bodyModel, glm::vec3(0.0f, bodyCenterY , 0.0f));
    bodyModel = glm::scale(bodyModel, glm::vec3(0.8f, 0.7f * jake->legStretch, 0.6f)); // Stretch body vertically too
    drawShape(cubeVAO, cubeVertexCount, bodyModel, COLOR_JAKE_BODY);

    // Head (Positioned relative to top of stretched body)
    glm::mat4 headModel = jakeModelBase;
     // Calculate top of the body including stretch
    float bodyTopY = JAKE_BASE_LEG_LENGTH * jake->legStretch + 0.7f; // Approx top of stretched body block
    headModel = glm::translate(headModel, glm::vec3(0.0f, bodyTopY + 0.25f, 0.1f)); // Position head above stretched body
    headModel = glm::rotate(headModel, jake->headInclination, glm::vec3(1.0f, 0.0f, 0.0f));
    headModel = glm::scale(headModel, glm::vec3(0.5f, 0.5f, 0.5f));
    drawShape(cubeVAO, cubeVertexCount, headModel, COLOR_JAKE_BODY);

    // Legs - Scale Y by legStretch, apply swing
    float legCenterY = 0.5f * JAKE_BASE_LEG_LENGTH * jake->legStretch; // Y center of stretched leg
    float legPivotY = JAKE_BASE_LEG_LENGTH * jake->legStretch; // Pivot point at top of leg

    glm::mat4 legLModel = jakeModelBase;
    legLModel = glm::translate(legLModel, glm::vec3(-0.2f, 0.0f, 0.0f)); // Base position
    legLModel = glm::translate(legLModel, glm::vec3(0.0f, legPivotY, 0.0f)); // Move to pivot
    legLModel = glm::rotate(legLModel, jake->legSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate
    legLModel = glm::translate(legLModel, glm::vec3(0.0f, -legCenterY, 0.0f)); // Move origin to center of stretched leg
    legLModel = glm::scale(legLModel, glm::vec3(0.3f, JAKE_BASE_LEG_LENGTH * jake->legStretch, 0.3f)); // Scale stretched leg
    drawShape(cubeVAO, cubeVertexCount, legLModel, COLOR_JAKE_BODY);

    glm::mat4 legRModel = jakeModelBase;
    legRModel = glm::translate(legRModel, glm::vec3(0.2f, 0.0f, 0.0f)); // Base position
    legRModel = glm::translate(legRModel, glm::vec3(0.0f, legPivotY, 0.0f)); // Move to pivot
    legRModel = glm::rotate(legRModel, -jake->legSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate opposite
    legRModel = glm::translate(legRModel, glm::vec3(0.0f, -legCenterY, 0.0f)); // Move origin to center
    legRModel = glm::scale(legRModel, glm::vec3(0.3f, JAKE_BASE_LEG_LENGTH * jake->legStretch, 0.3f)); // Scale stretched leg
    drawShape(cubeVAO, cubeVertexCount, legRModel, COLOR_JAKE_BODY);

    // Arms - Position relative to stretched body, apply swing
    float armAttachY = legPivotY + 0.3f; // Attach point slightly above leg tops
    float armCenterOffsetY = -0.3f; // Offset from attach point to arm center

    glm::mat4 armLModel = jakeModelBase;
    armLModel = glm::translate(armLModel, glm::vec3(-0.5f, armAttachY, 0.0f)); // Attach point
    armLModel = glm::rotate(armLModel, jake->armSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate
    armLModel = glm::translate(armLModel, glm::vec3(0.0f, armCenterOffsetY, 0.0f)); // Move origin to center
    armLModel = glm::scale(armLModel, glm::vec3(0.2f, 0.6f, 0.2f)); // Scale arm
    drawShape(cubeVAO, cubeVertexCount, armLModel, COLOR_JAKE_BODY);

    glm::mat4 armRModel = jakeModelBase;
    armRModel = glm::translate(armRModel, glm::vec3(0.5f, armAttachY, 0.0f)); // Attach point
    armRModel = glm::rotate(armRModel, -jake->armSwingAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate opposite
    armRModel = glm::translate(armRModel, glm::vec3(0.0f, armCenterOffsetY, 0.0f)); // Move origin to center
    armRModel = glm::scale(armRModel, glm::vec3(0.2f, 0.6f, 0.2f)); // Scale arm
    drawShape(cubeVAO, cubeVertexCount, armRModel, COLOR_JAKE_BODY);
}

void drawBMO(BMO* bmo, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 bmoModel = glm::mat4(1.0f);
    // BMO's origin should be at its base for ground placement
    bmoModel = glm::translate(bmoModel, bmo->position + glm::vec3(0.0f, 0.4f, 0.0f)); // Center Y relative to base
    bmoModel = glm::rotate(bmoModel, bmo->rotation, glm::vec3(0.0f, 1.0f, 0.0f));

    // Main Body (Scale relative to the centered origin)
    glm::mat4 bodyActual = glm::scale(bmoModel, glm::vec3(0.6f, 0.8f, 0.3f));
    drawShape(cubeVAO, cubeVertexCount, bodyActual, COLOR_BMO_BODY);

    // Screen (relative to the MAIN body's model matrix 'bodyActual')
    glm::mat4 screenModel = glm::translate(bodyActual, glm::vec3(0.0f, 0.1f, 0.51f)); // Move forward from body center
    screenModel = glm::scale(screenModel, glm::vec3(0.7f, 0.6f, 0.05f)); // Scale relative to body scale
    drawShape(cubeVAO, cubeVertexCount, screenModel, COLOR_BMO_SCREEN);

    // Buttons (relative to the MAIN body's model matrix 'bodyActual')
    float btnRelSize = 0.15f; // Size relative to body's dimensions
    glm::mat4 btnRedModel = glm::translate(bodyActual, glm::vec3(0.35f, -0.3f, 0.51f));
    btnRedModel = glm::scale(btnRedModel, glm::vec3(btnRelSize, btnRelSize, 0.1f));
    drawShape(cubeVAO, cubeVertexCount, btnRedModel, COLOR_BMO_BUTTON_RED);

    glm::mat4 btnBlueModel = glm::translate(bodyActual, glm::vec3(-0.35f, -0.15f, 0.51f));
    btnBlueModel = glm::scale(btnBlueModel, glm::vec3(btnRelSize * 1.5f, btnRelSize, 0.1f)); // D-pad shape
    drawShape(cubeVAO, cubeVertexCount, btnBlueModel, COLOR_BMO_BUTTON_BLUE);

    glm::mat4 btnYlwModel = glm::translate(bodyActual, glm::vec3(-0.30f, -0.35f, 0.51f)); // Position adjusted
    btnYlwModel = glm::scale(btnYlwModel, glm::vec3(btnRelSize*0.8f, btnRelSize*0.8f, 0.1f));
    drawShape(cubeVAO, cubeVertexCount, btnYlwModel, COLOR_BMO_BUTTON_YELLOW);
}

void drawIceKing(IceKing* ik, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 ikModel = glm::mat4(1.0f);
    // Position origin near base for easier height management when flying
    ikModel = glm::translate(ikModel, ik->position + glm::vec3(0.0f, 0.75f, 0.0f)); // Mid-body Y approx
    ikModel = glm::rotate(ikModel, ik->rotation, glm::vec3(0.0f, 1.0f, 0.0f));

    // Body (Robe)
    glm::mat4 bodyModel = glm::translate(ikModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Centered at origin
    bodyModel = glm::scale(bodyModel, glm::vec3(0.8f, 1.5f, 0.8f));
    drawShape(cubeVAO, cubeVertexCount, bodyModel, COLOR_ICE_KING_BODY);

    // Head (placeholder, mostly covered) - relative to ikModel origin
    glm::mat4 headModel = glm::translate(ikModel, glm::vec3(0.0f, 1.1f, 0.0f)); // Above body center
    headModel = glm::scale(headModel, glm::vec3(0.5f, 0.5f, 0.5f));
    drawShape(cubeVAO, cubeVertexCount, headModel, COLOR_ICE_KING_BODY); // Use body color

    // Beard (Multiple parts for shape) - relative to ikModel origin
    glm::mat4 beard1 = glm::translate(ikModel, glm::vec3(0.0f, 0.6f, 0.3f)); // Front main, below head
    beard1 = glm::scale(beard1, glm::vec3(0.9f, 1.2f, 0.4f));
    drawShape(cubeVAO, cubeVertexCount, beard1, COLOR_ICE_KING_BEARD);
    glm::mat4 beard2 = glm::translate(ikModel, glm::vec3(0.0f, 0.1f, 0.4f)); // Lower front, extending down
    beard2 = glm::scale(beard2, glm::vec3(0.6f, 0.6f, 0.3f));
    drawShape(cubeVAO, cubeVertexCount, beard2, COLOR_ICE_KING_BEARD);

    // Nose - relative to ikModel origin
    glm::mat4 noseModel = glm::translate(ikModel, glm::vec3(0.0f, 1.0f, 0.2f)); // Positioned near head center Z
    noseModel = glm::rotate(noseModel, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Slight downward tilt
    noseModel = glm::translate(noseModel, glm::vec3(0.0f, 0.0f, 0.3f)); // Move tip forward
    noseModel = glm::scale(noseModel, glm::vec3(0.1f, 0.1f, 0.6f)); // Long and thin Z
    drawShape(cubeVAO, cubeVertexCount, noseModel, COLOR_ICE_KING_BODY); // Skin color

    // Crown Base - relative to ikModel origin
    glm::mat4 crownBase = glm::translate(ikModel, glm::vec3(0.0f, 1.4f, 0.0f)); // Above head
    crownBase = glm::scale(crownBase, glm::vec3(0.6f, 0.2f, 0.6f));
    drawShape(cubeVAO, cubeVertexCount, crownBase, COLOR_ICE_KING_CROWN);

    // Crown Gems - relative to crownBase position
    float gemSize = 0.1f;
    glm::mat4 gem1 = glm::translate(ikModel, glm::vec3(0.0f, 1.55f, 0.28f)); // Front Center, slightly higher than base top
    gem1 = glm::scale(gem1, glm::vec3(gemSize));
    drawShape(cubeVAO, cubeVertexCount, gem1, COLOR_ICE_KING_GEM);
    glm::mat4 gem2 = glm::translate(ikModel, glm::vec3(0.28f, 1.55f, 0.0f)); // Right Center
    gem2 = glm::scale(gem2, glm::vec3(gemSize));
    drawShape(cubeVAO, cubeVertexCount, gem2, COLOR_ICE_KING_GEM);
    glm::mat4 gem3 = glm::translate(ikModel, glm::vec3(-0.28f, 1.55f, 0.0f)); // Left Center
    gem3 = glm::scale(gem3, glm::vec3(gemSize));
    drawShape(cubeVAO, cubeVertexCount, gem3, COLOR_ICE_KING_GEM);
}

void drawPB(PrincessBubblegum* pb, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 pbModel = glm::mat4(1.0f);
    // Position origin at base
    pbModel = glm::translate(pbModel, pb->position);
    pbModel = glm::rotate(pbModel, pb->rotation, glm::vec3(0.0f, 1.0f, 0.0f));

    // Dress (Main Body) - Origin at base center
    glm::mat4 dressModel = glm::translate(pbModel, glm::vec3(0.0f, 0.9f, 0.0f)); // Center Y of dress block
    dressModel = glm::scale(dressModel, glm::vec3(0.6f, 1.8f, 0.6f));
    drawShape(cubeVAO, cubeVertexCount, dressModel, COLOR_PB_DRESS);

    // Head - relative to pbModel origin
    glm::mat4 headModel = glm::translate(pbModel, glm::vec3(0.0f, 2.0f, 0.0f)); // Positioned above dress top
    headModel = glm::scale(headModel, glm::vec3(0.5f, 0.5f, 0.5f));
    drawShape(cubeVAO, cubeVertexCount, headModel, COLOR_PB_SKIN);

    // Hair (Simplified - blocks relative to head position)
    glm::mat4 hairBack = glm::translate(pbModel, glm::vec3(0.0f, 1.8f, -0.3f)); // Behind head, lower part
    hairBack = glm::scale(hairBack, glm::vec3(0.6f, 1.0f, 0.2f)); // Tall block down
    drawShape(cubeVAO, cubeVertexCount, hairBack, COLOR_PB_HAIR);
    glm::mat4 hairTop = glm::translate(pbModel, glm::vec3(0.0f, 2.1f, -0.1f)); // Top/frontish hair mass
    hairTop = glm::scale(hairTop, glm::vec3(0.6f, 0.4f, 0.6f));
    drawShape(cubeVAO, cubeVertexCount, hairTop, COLOR_PB_HAIR);

    // Crown - relative to pbModel origin
    glm::mat4 crownBase = glm::translate(pbModel, glm::vec3(0.0f, 2.3f, 0.0f)); // Above head
    crownBase = glm::scale(crownBase, glm::vec3(0.3f, 0.1f, 0.3f)); // Smaller crown
    drawShape(cubeVAO, cubeVertexCount, crownBase, COLOR_PB_CROWN);
    // Crown Gem - relative to crown position
    glm::mat4 gem = glm::translate(pbModel, glm::vec3(0.0f, 2.38f, 0.14f)); // Single front gem, slightly above base top
    gem = glm::scale(gem, glm::vec3(0.08f));
    drawShape(cubeVAO, cubeVertexCount, gem, COLOR_PB_GEM);

    // Simple Arms (Optional) - relative to pbModel origin
    glm::mat4 armL = glm::translate(pbModel, glm::vec3(-0.4f, 1.4f, 0.0f)); // Shoulder height approx
    armL = glm::translate(armL, glm::vec3(0.0f, -0.4f, 0.0f)); // Center of arm
    armL = glm::scale(armL, glm::vec3(0.15f, 0.8f, 0.15f));
    drawShape(cubeVAO, cubeVertexCount, armL, COLOR_PB_SKIN);
    glm::mat4 armR = glm::translate(pbModel, glm::vec3(0.4f, 1.4f, 0.0f)); // Shoulder
    armR = glm::translate(armR, glm::vec3(0.0f, -0.4f, 0.0f)); // Center
    armR = glm::scale(armR, glm::vec3(0.15f, 0.8f, 0.15f));
    drawShape(cubeVAO, cubeVertexCount, armR, COLOR_PB_SKIN);
}


void drawMarceline(Marceline* marcy, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 marcyModel = glm::mat4(1.0f);
    // Position origin at base/feet
    marcyModel = glm::translate(marcyModel, marcy->position);
    marcyModel = glm::rotate(marcyModel, marcy->rotation, glm::vec3(0.0f, 1.0f, 0.0f));

    // Legs/Pants - Origin at center base
    glm::mat4 legL = glm::translate(marcyModel, glm::vec3(-0.15f, 0.5f, 0.0f)); // Center Y of leg block
    legL = glm::scale(legL, glm::vec3(0.2f, 1.0f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, legL, COLOR_MARCELINE_PANTS);
    glm::mat4 legR = glm::translate(marcyModel, glm::vec3(0.15f, 0.5f, 0.0f)); // Center Y
    legR = glm::scale(legR, glm::vec3(0.2f, 1.0f, 0.2f));
    drawShape(cubeVAO, cubeVertexCount, legR, COLOR_MARCELINE_PANTS);

    // Body (Shirt) - Above legs
    glm::mat4 bodyModel = glm::translate(marcyModel, glm::vec3(0.0f, 1.4f, 0.0f)); // Center Y of body block
    bodyModel = glm::scale(bodyModel, glm::vec3(0.5f, 0.8f, 0.3f));
    drawShape(cubeVAO, cubeVertexCount, bodyModel, COLOR_MARCELINE_SHIRT);

    // Head - Above body
    glm::mat4 headModel = glm::translate(marcyModel, glm::vec3(0.0f, 2.0f, 0.0f)); // Center Y of head
    headModel = glm::scale(headModel, glm::vec3(0.5f, 0.5f, 0.5f));
    drawShape(cubeVAO, cubeVertexCount, headModel, COLOR_MARCELINE_SKIN);

    // Hair (Very Long - multiple blocks relative to marcyModel origin)
    glm::mat4 hair1 = glm::translate(marcyModel, glm::vec3(0.0f, 1.2f, -0.2f)); // Back, covering body/head transition
    hair1 = glm::scale(hair1, glm::vec3(0.6f, 2.0f, 0.3f)); // Long block
    drawShape(cubeVAO, cubeVertexCount, hair1, COLOR_MARCELINE_HAIR);
    glm::mat4 hair2 = glm::translate(marcyModel, glm::vec3(0.0f, 0.0f, -0.3f)); // Lower back, near ground
    hair2 = glm::scale(hair2, glm::vec3(0.5f, 1.0f, 0.3f));
    drawShape(cubeVAO, cubeVertexCount, hair2, COLOR_MARCELINE_HAIR);

    // Simple Arms - Relative to marcyModel origin
    glm::mat4 armL = glm::translate(marcyModel, glm::vec3(-0.4f, 1.4f, 0.0f)); // Shoulder height
    armL = glm::translate(armL, glm::vec3(0.0f, -0.4f, 0.0f)); // Center arm
    armL = glm::scale(armL, glm::vec3(0.15f, 0.8f, 0.15f));
    drawShape(cubeVAO, cubeVertexCount, armL, COLOR_MARCELINE_SKIN);
    glm::mat4 armR = glm::translate(marcyModel, glm::vec3(0.4f, 1.4f, 0.0f)); // Shoulder height
    armR = glm::translate(armR, glm::vec3(0.0f, -0.4f, 0.0f)); // Center arm
    armR = glm::scale(armR, glm::vec3(0.15f, 0.8f, 0.15f));
    drawShape(cubeVAO, cubeVertexCount, armR, COLOR_MARCELINE_SKIN);

    // Bass Guitar (Optional - simple representation)
    // glm::mat4 bassBody = glm::translate(marcyModel, glm::vec3(-0.3f, 1.0f, 0.3f)); // Held position ~waist height
    // bassBody = glm::rotate(bassBody, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Angle across body
    // bassBody = glm::rotate(bassBody, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Slight angle out
    // bassBody = glm::scale(bassBody, glm::vec3(0.4f, 1.0f, 0.1f)); // Axe shape?
    // drawShape(cubeVAO, cubeVertexCount, bassBody, COLOR_MARCELINE_BASS);
    // glm::mat4 bassNeck = glm::translate(bassBody, glm::vec3(0.0f, 0.8f, 0.0f)); // Extend neck from body center upwards
    // bassNeck = glm::scale(bassNeck, glm::vec3(0.1f/0.4f, 1.0f/1.0f, 0.1f/0.1f)); // Counter-act body scale
    // bassNeck = glm::scale(bassNeck, glm::vec3(0.08f, 1.2f, 0.08f)); // Actual neck dimensions
    // drawShape(cubeVAO, cubeVertexCount, bassNeck, COLOR_SWORD_GREY); // Neck color
}


// --- Função Principal ---
int main() {
    // --- Inicialização GLFW, Janela, GLEW (Inalterado) ---
    if (!glfwInit()) { std::cerr << "Failed to initialize GLFW" << std::endl; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Adventure Time Modern OpenGL - More Characters!", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window" << std::endl; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    // Vsync (optional)
    glfwSwapInterval(1);

    // GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) { std::cerr << "Failed to initialize GLEW" << std::endl; glfwTerminate(); return -1; }

    // --- Shaders, Geometrias ---
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0) { glfwTerminate(); return -1; } // Check for shader errors

    std::vector<glm::vec3> cubePositions = generateCubePositions();
    setupGeometry(cubeVAO, cubeVBO, cubePositions, cubeVertexCount);
    std::vector<glm::vec3> pyramidPositions = generatePyramidPositions();
    setupGeometry(pyramidVAO, pyramidVBO, pyramidPositions, pyramidVertexCount);
    std::vector<glm::vec3> conePositions = generateConePositions();
    setupGeometry(coneVAO, coneVBO, conePositions, coneVertexCount);

    // --- Config OpenGL ---
    glEnable(GL_DEPTH_TEST);
    glClearColor(COLOR_SKY_BLUE.r, COLOR_SKY_BLUE.g, COLOR_SKY_BLUE.b, 1.0f);
    glEnable(GL_CULL_FACE); // Cull back faces for potentially better performance
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Assuming standard counter-clockwise winding

    // --- Personagens ---
    Finn* finn = new Finn(glm::vec3(-5.0f, 0.0f, 5.0f)); // Start further left, slightly forward
    Jake* jake = new Jake(glm::vec3(5.0f, 0.0f, 5.0f));   // Start further right, slightly forward

    // NPCs
    std::vector<Character*> npcs;
    npcs.push_back(new BMO(glm::vec3(0.0f, 0.0f, -5.0f)));
    npcs.push_back(new PrincessBubblegum(glm::vec3(-5.0f, 0.0f, -10.0f)));
    npcs.push_back(new IceKing(glm::vec3(0.0f, 5.0f, -15.0f))); // Start flying
    npcs.push_back(new Marceline(glm::vec3(5.0f, 4.0f, -8.0f)));  // Start flying

    // Vector containing ALL controllable characters
    std::vector<Character*> allCharacters;
    allCharacters.push_back(finn); // Index 0
    allCharacters.push_back(jake); // Index 1
    allCharacters.insert(allCharacters.end(), npcs.begin(), npcs.end()); // Indices 2, 3, 4, 5...

    // Other scene objects
    glm::vec3 pyramidPos(15.0f, 0.0f, -15.0f);
    glm::vec3 conePos(-15.0f, 0.0f, -15.0f);
    float coneScaleFactor = 1.5f;

    int activeCharacterIndex = 0; // Index in allCharacters vector
    double lastTime = glfwGetTime();

    // --- Loop Principal ---
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        // Clamp deltaTime to avoid large jumps if debugging or window hangs
        deltaTime = std::min(deltaTime, 0.1f);


        // --- Processamento de Entrada ---
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        // Character Selection (Keys '1' through 'N')
        for (int i = 0; i < allCharacters.size(); ++i) {
            if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS) {
                activeCharacterIndex = i;
            }
        }
        // Ensure index is valid (safety check)
        if (activeCharacterIndex < 0 || activeCharacterIndex >= allCharacters.size()) {
            activeCharacterIndex = 0; // Default to Finn if something went wrong
        }

        // Wireframe Toggle
        bool zeroKeyPressed = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;
        if (zeroKeyPressed && !zeroKeyPressedLastFrame) wireframeMode = !wireframeMode;
        zeroKeyPressedLastFrame = zeroKeyPressed;

        // Get the currently controlled character
        Character* controlledChar = allCharacters[activeCharacterIndex];

        // Movement and Actions for the controlled character
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) controlledChar->moveForward(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) controlledChar->moveBackward(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) controlledChar->rotateLeft(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) controlledChar->rotateRight(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) controlledChar->headInclination = std::min(controlledChar->headInclination + 2.0f * deltaTime, glm::pi<float>() / 4.0f);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) controlledChar->headInclination = std::max(controlledChar->headInclination - 2.0f * deltaTime, -glm::pi<float>() / 4.0f);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            controlledChar->startJump(controlledChar->initialJumpSpeed); // Use character's own jump speed
        }

        // Character Specific Actions (E for Finn, O/P for Jake)
        if (Finn* finnPtr = dynamic_cast<Finn*>(controlledChar)) {
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) finnPtr->startAttack();
        } else if (Jake* jakePtr = dynamic_cast<Jake*>(controlledChar)) {
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) jakePtr->legStretch = 3.0f;
            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) jakePtr->sizeMultiplier = 2.5f;
        }

        // Cone Scaling (I/K)
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) coneScaleFactor += 1.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) coneScaleFactor -= 1.0f * deltaTime; coneScaleFactor = std::max(0.1f, coneScaleFactor);


        // --- Atualizações ---

        // Set player control flag before updating
        for (size_t i = 0; i < allCharacters.size(); ++i) {
            allCharacters[i]->isUnderPlayerControl = (i == activeCharacterIndex);
        }

        // Update ALL characters (base update handles player control vs NPC wander)
        for (Character* character : allCharacters) {
            character->update(deltaTime);
        }


        // --- Lógica de Seguir (Only Finn and Jake follow each other) ---
        if (activeCharacterIndex == 0 || activeCharacterIndex == 1) { // Only if Finn or Jake is controlled
            Character* leader = controlledChar; // The one being controlled
            Character* follower = (activeCharacterIndex == 0) ? (Character*)jake : (Character*)finn; // The other one

            // Don't let the follower wander if it's being followed
            follower->isUnderPlayerControl = false; // Ensure wander logic *could* run if far away
                                                  // But follower logic below will override position

            glm::vec3 directionToLeader = leader->position - follower->position;
            float distance = glm::length(directionToLeader);
            float desiredDistance = 3.0f; // How far follower stays behind
            float followSpeedMultiplier = 0.8f; // Slower than leader speed

            // Only move if not too close and leader isn't follower (safety)
            if (distance > desiredDistance && leader != follower) {
                glm::vec3 moveDir = glm::normalize(directionToLeader);

                // Make follower face the leader
                follower->rotation = atan2(moveDir.x, moveDir.z);

                // Move follower towards a point behind the leader
                glm::vec3 targetFollowPos = leader->position - moveDir * desiredDistance;
                glm::vec3 moveToTargetDir = targetFollowPos - follower->position;

                // Move only if significantly far from target follow position
                if (glm::length(moveToTargetDir) > 0.5f) {
                     // Use follower's speed, potentially adjusted
                     float effectiveFollowSpeed = follower->speed * followSpeedMultiplier;
                     // Check if follower is Jake to potentially adjust speed (optional)
                     // if (dynamic_cast<Jake*>(follower)) { effectiveFollowSpeed *= 0.9f; }

                    // Use normalized direction towards target follow pos
                    follower->position += glm::normalize(moveToTargetDir) * effectiveFollowSpeed * deltaTime;
                    follower->moving = true; // Indicate movement for animation

                     // Ensure follower stays on ground if not a flyer and not jumping
                    if (!dynamic_cast<IceKing*>(follower) && !dynamic_cast<Marceline*>(follower) && !follower->isJumping) {
                        // Check if follower is Jake to use effective ground height
                         float targetGround = follower->groundHeight;
                         if (Jake* jFollower = dynamic_cast<Jake*>(follower)) {
                             targetGround = jFollower->getEffectiveGroundHeight();
                         }
                         follower->position.y = targetGround;
                    }
                } else {
                     follower->moving = false;
                }
            } else {
                 follower->moving = false; // Stop follower animation if close
            }
        } // End Finn/Jake follow logic


        // --- Renderização ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Matrizes View/Projection (Camera adjusted slightly)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 150.0f); // Increased far plane
        glm::vec3 cameraPos = glm::vec3(0.0f, 8.0f, 35.0f); // Pulled back further, slightly higher
        glm::vec3 cameraTarget = glm::vec3(0.0f, 2.0f, 0.0f); // Look slightly lower
        // Simple camera orbit around target (optional)
        // float camX = sin(glfwGetTime() * 0.1f) * 35.0f;
        // float camZ = cos(glfwGetTime() * 0.1f) * 35.0f;
        // cameraPos = glm::vec3(camX, 8.0f, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // --- Desenhar Objetos ---
        // Chão (Larger)
        glm::mat4 groundModel = glm::mat4(1.0f);
        groundModel = glm::translate(groundModel, glm::vec3(0.0f, -0.5f, 0.0f));
        groundModel = glm::scale(groundModel, glm::vec3(GROUND_SIZE, 1.0f, GROUND_SIZE));
        drawShape(cubeVAO, cubeVertexCount, groundModel, COLOR_GRASS_GREEN);

        // Pirâmide (Optional)
        glm::mat4 pyramidModel = glm::mat4(1.0f);
        pyramidModel = glm::translate(pyramidModel, glm::vec3(pyramidPos.x, pyramidPos.y + 1.0f, pyramidPos.z)); // Adjusted base Y
        pyramidModel = glm::scale(pyramidModel, glm::vec3(2.0f, 2.0f, 2.0f));
        // drawShape(pyramidVAO, pyramidVertexCount, pyramidModel, glm::vec3(0.8f, 0.2f, 0.5f)); // Example color

        // Cone (Optional)
        glm::mat4 coneModel = glm::mat4(1.0f);
        coneModel = glm::translate(coneModel, glm::vec3(conePos.x, conePos.y + (coneScaleFactor * 1.5f)/2.0f - 0.5f, conePos.z)); // Adjusted base Y
        coneModel = glm::rotate(coneModel, (float)glfwGetTime() * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        coneModel = glm::scale(coneModel, glm::vec3(coneScaleFactor, coneScaleFactor * 1.5f, coneScaleFactor));
        // drawShape(coneVAO, coneVertexCount, coneModel, glm::vec3(0.5f, 0.2f, 0.8f)); // Example color


        // Draw ALL Characters - Use dynamic_cast to call correct draw function
        for (Character* character : allCharacters) {
            if (Finn* f = dynamic_cast<Finn*>(character)) { drawFinn(f, view, projection); }
            else if (Jake* j = dynamic_cast<Jake*>(character)) { drawJake(j, view, projection); }
            else if (BMO* b = dynamic_cast<BMO*>(character)) { drawBMO(b, view, projection); }
            else if (PrincessBubblegum* p = dynamic_cast<PrincessBubblegum*>(character)) { drawPB(p, view, projection); }
            else if (IceKing* i = dynamic_cast<IceKing*>(character)) { drawIceKing(i, view, projection); }
            else if (Marceline* m = dynamic_cast<Marceline*>(character)) { drawMarceline(m, view, projection); }
            // else draw generic placeholder?
        }

        // --- Swap Buffers & Poll Events ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Limpeza ---
    glDeleteVertexArrays(1, &cubeVAO); glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &pyramidVAO); glDeleteBuffers(1, &pyramidVBO);
    glDeleteVertexArrays(1, &coneVAO); glDeleteBuffers(1, &coneVBO);
    glDeleteProgram(shaderProgram);

    // Delete all characters allocated with new
    for (Character* character : allCharacters) {
        delete character;
    }
    allCharacters.clear(); // Clear the vector pointers
    // No need to clear npcs separately as its contents were moved to allCharacters


    glfwTerminate();
    return 0;
}

// --- Implementações Faltantes (OpenGL Helpers, Geometry) ---

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shader); // Clean up failed shader
        return 0; // Return 0 on failure
    }
    return shader;
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    // Detach and delete shaders after linking
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

std::vector<glm::vec3> generateCubePositions() {
     return {
        // Frente (+Z) - CCW
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3( 0.5f, -0.5f, 0.5f), glm::vec3( 0.5f,  0.5f, 0.5f),
        glm::vec3( 0.5f,  0.5f, 0.5f), glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
        // Trás (-Z) - CCW
        glm::vec3( 0.5f, -0.5f,-0.5f), glm::vec3(-0.5f, -0.5f,-0.5f), glm::vec3(-0.5f,  0.5f,-0.5f),
        glm::vec3(-0.5f,  0.5f,-0.5f), glm::vec3( 0.5f,  0.5f,-0.5f), glm::vec3( 0.5f, -0.5f,-0.5f),
        // Esquerda (-X) - CCW
        glm::vec3(-0.5f, -0.5f,-0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f,  0.5f, 0.5f),
        glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(-0.5f,  0.5f,-0.5f), glm::vec3(-0.5f, -0.5f,-0.5f),
        // Direita (+X) - CCW
        glm::vec3( 0.5f, -0.5f, 0.5f), glm::vec3( 0.5f, -0.5f,-0.5f), glm::vec3( 0.5f,  0.5f,-0.5f),
        glm::vec3( 0.5f,  0.5f,-0.5f), glm::vec3( 0.5f,  0.5f, 0.5f), glm::vec3( 0.5f, -0.5f, 0.5f),
        // Baixo (-Y) - CCW
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f,-0.5f), glm::vec3( 0.5f, -0.5f,-0.5f),
        glm::vec3( 0.5f, -0.5f,-0.5f), glm::vec3( 0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
        // Cima (+Y) - CCW
        glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3( 0.5f,  0.5f, 0.5f), glm::vec3( 0.5f,  0.5f,-0.5f),
        glm::vec3( 0.5f,  0.5f,-0.5f), glm::vec3(-0.5f,  0.5f,-0.5f), glm::vec3(-0.5f,  0.5f, 0.5f),
    };
}

std::vector<glm::vec3> generatePyramidPositions() {
    return {
        // Faces laterais (CCW from outside)
        glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), // Frente
        glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), // Direita
        glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), // Trás
        glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), // Esquerda
        // Base (CCW from top looking down)
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), // Base 1
        glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), // Base 2
    };
}

std::vector<glm::vec3> generateConePositions(int slices) {
    std::vector<glm::vec3> vertices;
    glm::vec3 tip(0.0f, 0.5f, 0.0f);
    glm::vec3 baseCenter(0.0f, -0.5f, 0.0f);
    float radius = 0.5f;
    float angleStep = 2.0f * glm::pi<float>() / slices;

    for (int i = 0; i < slices; ++i) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
         // Explicitly handle wrap-around for vertex positions to avoid tiny gaps
        glm::vec3 p1(radius * cos(angle1), -0.5f, radius * sin(angle1));
        glm::vec3 p2(radius * cos(angle2), -0.5f, radius * sin(angle2));

        // Lados (Triângulo: topo, base_i, base_i+1) - CCW from outside
        vertices.push_back(tip);
        vertices.push_back(p1);
        vertices.push_back(p2);

        // Base (Triângulo: centro, base_i+1, base_i) - CCW from top (looking down)
        vertices.push_back(baseCenter);
        vertices.push_back(p2);
        vertices.push_back(p1);
    }
    return vertices;
}


void setupGeometry(GLuint& vao, GLuint& vbo, const std::vector<glm::vec3>& positions, GLsizei& vertexCount) {
    vertexCount = static_cast<GLsizei>(positions.size());
    if (vertexCount == 0) return;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

    // Atributo Posição (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawShape(GLuint vao, GLsizei vertexCount, glm::mat4 model, const glm::vec3& color) {
    if (vertexCount == 0 || vao == 0) return;

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);

    // Reset polygon mode to default if you changed it
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Optional reset
}