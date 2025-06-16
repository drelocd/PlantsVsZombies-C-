// zombie.h
#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "raylib.h"
#include <vector>
#include <memory> // For std::unique_ptr
#include "game_constants.h"

// Forward declaration for Plant
class Plant;

// Enum to differentiate zombie types
enum class ZombieType {
    REGULAR,
    JUMPING
};

//----------------------------------------------------------------------------------
// Base Zombie Class
//----------------------------------------------------------------------------------
class Zombie {
public:
    // Reordered members to match the constructor initialization order in zombie.cpp
    Rectangle rect;
    int health;
    float speed;
    bool active;
    Color color; // Fallback color, will be overridden by texture
    Texture2D texture; // Sprite sheet for the zombie
    Rectangle sourceRect; // Current frame in the sprite sheet
    int currentFrame;
    float frameTimer;
    float frameSpeed;
    int numFrames;         // Total horizontal frames in *one* row
    int numSpriteRows;     // Total number of rows in the sprite sheet
    int currentRowIndex;   // Which row to animate from (0 for top, 1 for next, etc.)
    int row;
    bool isAttacking;      // True if currently eating a plant
    float biteTimer;
    float biteRate;        // Time between bites (e.g., 0.5s per bite)
    int attackDamagePerBite; // Damage dealt per bite
    int scoreValue;        // Added this based on our previous discussion!

    // --- NEW MEMBERS FOR SLOW EFFECT ---
    bool isSlowed;
    float slowTimer;
    float originalSpeed;
    // -----------------------------------

    // UPDATED: Added 'int level' parameter to the constructor
    Zombie(Rectangle rect, int baseHealth, float baseSpeed, Color color, Texture2D tex, int row,
           int numFrames, float frameSpeed, int numSpriteRows, int currentRowIndex,
           int attackDamagePerBite, float biteRate, int scoreValue, int level);

    virtual ~Zombie() = default;

    virtual void Update(float deltaTime, std::vector<std::unique_ptr<Plant>>& plants) = 0;
    virtual void Draw() const;
    virtual ZombieType GetType() const = 0;

    void TakeDamage(int damage) { // Common function for all zombies
        health -= damage;
        if (health <= 0) {
            active = false;
        }
    }

    void AttackPlant(Plant* plant, float deltaTime);
    void UpdateSourceRect();

    // --- NEW FUNCTION FOR SLOW EFFECT ---
    void ApplySlowEffect();
    // ------------------------------------
};

//----------------------------------------------------------------------------------
// Derived Zombie Classes
//----------------------------------------------------------------------------------

// RegularZombie
class RegularZombie : public Zombie {
public:
    RegularZombie(Rectangle rect, int row, Texture2D tex, int level);
    void Update(float deltaTime, std::vector<std::unique_ptr<Plant>>& plants) override;
    ZombieType GetType() const override { return ZombieType::REGULAR; }
};

// JumpingZombie
class JumpingZombie : public Zombie {
private:
    bool isJumping;
    float jumpTimer;
    float jumpDuration;
    float initialY;
    float jumpPeakHeight;

public:
    JumpingZombie(Rectangle rect, int row, Texture2D tex, int level);
    void Update(float deltaTime, std::vector<std::unique_ptr<Plant>>& plants) override;
    ZombieType GetType() const override { return ZombieType::JUMPING; }
};

#endif // ZOMBIE_H