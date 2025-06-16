// zombie.cpp (FINAL CORRECTED VERSION)

#include "zombie.h"
#include "plant.h" // Needed to interact with Plant objects
#include "game_constants.h" // Include game_constants.h for all constants
#include <iostream>
#include <cmath> // For std::pow if you use exponential scaling

// REMOVED REDUNDANT EXTERN DECLARATIONS HERE.
// These are already declared in game_constants.h as extern.
// You only define them in main.cpp.

//----------------------------------------------------------------------------------
// Base Zombie Implementation
//----------------------------------------------------------------------------------
Zombie::Zombie(Rectangle rect, int baseHealth, float baseSpeed, Color color, Texture2D tex, int row,
               int numFrames, float frameSpeed, int numSpriteRows, int currentRowIndex,
               int attackDamagePerBite_param, float biteRate_param, int scoreValue_param, int level)
    // Initialize members in the SAME ORDER as they are declared in zombie.h to avoid -Wreorder
    : rect(rect),
      health(static_cast<int>(baseHealth * (1.0f + (level - 1) * 0.2f))), // Apply level scaling to health
      speed(baseSpeed + (level - 1) * 2.0f), // Example: Speed scales by 2.0f per level
      active(true),
      color(color),
      texture(tex),
      sourceRect({0, 0, (float)tex.width / numFrames, (float)tex.height / numSpriteRows}), // Calculated here
      currentFrame(0),
      frameTimer(0.0f),
      frameSpeed(frameSpeed),
      numFrames(numFrames),
      numSpriteRows(numSpriteRows),
      currentRowIndex(currentRowIndex),
      row(row), // Reordered to match header (if it was after animation params)
      isAttacking(false),
      biteTimer(0.0f),
      biteRate(biteRate_param),
      attackDamagePerBite(attackDamagePerBite_param + (level - 1) * 5), // Damage scales with level
      scoreValue(scoreValue_param), // Base score value, not scaled with level here as per comment
      // --- INITIALIZE NEW MEMBERS FOR SLOW EFFECT ---
      isSlowed(false),
      slowTimer(0.0f),
      originalSpeed(baseSpeed + (level - 1) * 2.0f) // IMPORTANT: Initialize with the calculated base speed
{
    // Ensure health doesn't drop below 1
    if (health < 1) health = 1;
    // Ensure speed doesn't go below zero if you have very high levels
    if (speed < 0.0f) speed = 0.0f;

    // No need to set sourceRect again here, it's done in the initializer list
}

void Zombie::Draw() const {
    if (active) {
        DrawTexturePro(texture, sourceRect, rect, {0, 0}, 0, WHITE);
        // Optional: Draw health bar for debugging
        // You would need to pass an initial/max health value to the Zombie class
        // to correctly draw a health bar, or calculate it here based on level.
        // For example:
        // int maxHealthAtThisLevel = static_cast<int>(initialBaseHealth * (1.0f + (level - 1) * 0.2f));
        // DrawRectangle(rect.x, rect.y - 10, rect.width * (health / (float)maxHealthAtThisLevel), 5, GREEN);
    }
}

void Zombie::UpdateSourceRect() {
    float singleFrameWidth = (float)texture.width / numFrames;
    float singleFrameHeight = (float)texture.height / numSpriteRows;

    sourceRect.x = currentFrame * singleFrameWidth;
    sourceRect.y = currentRowIndex * singleFrameHeight;
    sourceRect.width = singleFrameWidth;
    sourceRect.height = singleFrameHeight;
}

void Zombie::AttackPlant(Plant* plant, float deltaTime) {
    isAttacking = true;
    biteTimer += deltaTime;

    // Animation update during attack
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        UpdateSourceRect();
    }

    if (biteTimer >= biteRate) {
        biteTimer = 0.0f;
        plant->TakeDamage(attackDamagePerBite);
    }
}

// --- Implementation of ApplySlowEffect ---
void Zombie::ApplySlowEffect() {
    if (!isSlowed) { // Only apply if not already slowed
        originalSpeed = speed; // Store current speed before slowing
        speed *= 0.5f; // Reduce speed by 50% (or your desired slow amount)
        isSlowed = true;
        slowTimer = 3.0f; // Set a duration for the slow effect (e.g., 3 seconds)
        // std::cout << "Zombie slowed! Current speed: " << speed << std::endl; // Debug
    }
}

//----------------------------------------------------------------------------------
// RegularZombie Implementation
//----------------------------------------------------------------------------------
RegularZombie::RegularZombie(Rectangle rect, int row, Texture2D tex, int level)
    : Zombie(rect,
             REGULAR_ZOMBIE_HEALTH, REGULAR_ZOMBIE_SPEED, RED, tex, row,
             REGULAR_ZOMBIE_WALKING_NUM_FRAMES, REGULAR_ZOMBIE_WALKING_FRAME_SPEED,
             REGULAR_ZOMBIE_TOTAL_SPRITE_ROWS,
             0, // currentRowIndex for walking (assuming row 0 for walking animation)
             ZOMBIE_DAMAGE_PER_BITE, ZOMBIE_BITE_RATE, // Base bite damage and rate
             REGULAR_ZOMBIE_SCORE_VALUE, // Base score value of 100 is passed here
             level) // Pass the 'level' here!
{
    // No specific initialization needed here, base constructor handles health calculation and scaling
}

void RegularZombie::Update(float deltaTime, std::vector<std::unique_ptr<Plant>>& plants) {
    if (!active) return;

    // --- SLOW EFFECT LOGIC (MUST BE INCLUDED IN EACH DERIVED UPDATE) ---
    if (isSlowed) {
        slowTimer -= deltaTime;
        if (slowTimer <= 0) {
            speed = originalSpeed; // Restore original speed
            isSlowed = false;
        }
    }
    // -----------------------------------------------------------------

    bool wasAttacking = isAttacking;
    isAttacking = false; // Reset attack state for current frame

    // Check for collision with plants in the same row
    for (auto& plant : plants) {
        if (plant->active && plant->row == this->row && CheckCollisionRecs(this->rect, plant->rect)) {
            AttackPlant(plant.get(), deltaTime);
            isAttacking = true; // Set to true if collision and attack happened
            break; // Stop checking after first collision, attack only one plant at a time
        }
    }

    // Animation state transition logic for Regular Zombie
    if (isAttacking) {
        // If we just started attacking OR we were walking previously, switch to eating animation
        // currentRowIndex 1 is assumed for eating, 0 for walking
        if (!wasAttacking || this->currentRowIndex != 1) {
            this->currentRowIndex = 1;
            this->numFrames = REGULAR_ZOMBIE_EATING_NUM_FRAMES;
            this->frameSpeed = REGULAR_ZOMBIE_EATING_FRAME_SPEED;
            this->currentFrame = 0; // Reset frame to start of eating animation
            this->frameTimer = 0.0f;
            UpdateSourceRect(); // Update sourceRect immediately on state change
        }
        // Zombie doesn't move forward while attacking
    } else { // Not attacking (i.e., moving)
        // If we just stopped attacking OR we were eating previously, switch to walking animation
        if (wasAttacking || this->currentRowIndex != 0) {
            this->currentRowIndex = 0;
            this->numFrames = REGULAR_ZOMBIE_WALKING_NUM_FRAMES;
            this->frameSpeed = REGULAR_ZOMBIE_WALKING_FRAME_SPEED;
            this->currentFrame = 0; // Reset frame to start of walking animation
            this->frameTimer = 0.0f;
            UpdateSourceRect(); // Update sourceRect immediately on state change
        }

        // Only move if not attacking (and apply current speed, whether normal or slowed)
        this->rect.x -= this->speed * deltaTime;
    }

    // Update animation frame (applies to both walking and eating states)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        UpdateSourceRect();
    }
}

//----------------------------------------------------------------------------------
// JumpingZombie Implementation
//----------------------------------------------------------------------------------
JumpingZombie::JumpingZombie(Rectangle rect, int row, Texture2D tex, int level)
    : Zombie(rect,
             JUMPING_ZOMBIE_HEALTH, JUMPING_ZOMBIE_SPEED, BLUE, tex, row, // BLUE for visual distinction
             JUMPING_ZOMBIE_NUM_FRAMES, JUMPING_ZOMBIE_FRAME_SPEED,
             JUMPING_ZOMBIE_TOTAL_SPRITE_ROWS,
             0, // Assuming initial currentRowIndex is 0 for jumping zombie
             ZOMBIE_DAMAGE_PER_BITE, ZOMBIE_BITE_RATE, // Base bite damage and rate (or specific jumping zombie ones)
             JUMPING_ZOMBIE_SCORE_VALUE, // Base score value for jumping zombie
             level), // Pass the 'level' here!
      // Initialize JumpingZombie specific members AFTER the base class constructor
      isJumping(false), jumpTimer(0.0f), jumpDuration(0.8f), initialY(rect.y), jumpPeakHeight(TILE_SIZE * 0.75f)
{
    // No specific initialization needed here, base constructor handles health calculation
}

void JumpingZombie::Update(float deltaTime, std::vector<std::unique_ptr<Plant>>& plants) {
    if (!active) return;

    // --- SLOW EFFECT LOGIC (MUST BE INCLUDED IN EACH DERIVED UPDATE) ---
    if (isSlowed) {
        slowTimer -= deltaTime;
        if (slowTimer <= 0) {
            speed = originalSpeed; // Restore original speed
            isSlowed = false;
        }
    }
    // -----------------------------------------------------------------

    isAttacking = false; // Reset attack state for current frame

    Plant* collidedPlant = nullptr;
    for (auto& plant : plants) {
        if (plant->active && plant->row == this->row && CheckCollisionRecs(this->rect, plant->rect)) {
            collidedPlant = plant.get();
            break; // Found a plant to interact with
        }
    }

    if (collidedPlant) {
        // Jumping Zombie logic: Jump over specific plants (Cherry Bomb, Wall-nut), attack others
        // Make sure PlantType is accessible (e.g., through plant.h or game_constants.h)
        if (collidedPlant->GetType() == PlantType::CHERRY_BOMB || collidedPlant->GetType() == PlantType::WALNUT) {
            if (!isJumping) {
                isJumping = true;
                jumpTimer = 0.0f;
                initialY = this->rect.y; // Store initial Y before jump starts
            }
        } else {
            // Attack other types of plants
            AttackPlant(collidedPlant, deltaTime);
            isAttacking = true; // Zombie is currently attacking
            isJumping = false; // Ensure not jumping if attacking
            this->rect.y = initialY; // Reset Y position if it was in a partial jump
            jumpTimer = 0.0f;
            // Don't move if attacking (handled by isAttacking check later)
        }
    }

    if (isJumping) {
        jumpTimer += deltaTime;
        float progress = jumpTimer / jumpDuration;

        if (progress >= 1.0f) {
            isJumping = false;
            this->rect.y = initialY; // Land back at original Y
            jumpTimer = 0.0f;
        } else {
            // Parabolic jump motion (yOffset is negative for upward movement)
            // This formula creates a nice arc: 4 * peak_height * progress * (1 - progress)
            float yOffset = -4 * jumpPeakHeight * progress * (1.0f - progress);
            this->rect.y = initialY + yOffset;
        }
        // Jumping zombies typically keep moving forward while jumping (apply current speed, normal or slowed)
        this->rect.x -= this->speed * deltaTime;

    } else if (!isAttacking) { // Only move if not jumping AND not attacking (apply current speed)
        this->rect.x -= this->speed * deltaTime;
    }

    // Update animation frame (for both moving/jumping and attacking states)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        UpdateSourceRect();
    }
}