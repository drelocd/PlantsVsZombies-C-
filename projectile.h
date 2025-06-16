// projectile.h
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h" // Needed for Rectangle, Vector2, Color, Texture2D
#include <vector>   // Needed for Projectile::Update interaction with zombies
#include <memory>   // Needed for std::unique_ptr

// Forward declaration for Zombie, as Projectile might interact with it
class Zombie;

// Define ProjectileType ENUM CLASS FIRST
// This directly fixes the "ProjectileType has not been declared" error.
enum class ProjectileType {
    NORMAL,
    FROZEN // For IcePea projectiles
};

//----------------------------------------------------------------------------------
// Game Object Structures (Projectile Class/Struct)
//----------------------------------------------------------------------------------

class Projectile { // Changed to class as it has methods and specific members
public:
    Rectangle rect;
    Vector2 speed;
    bool active;
    Color color; // Will be replaced by Texture2D for actual pea image
    Texture2D texture;
    Rectangle sourceRect;
    int currentFrame;
    float frameTimer;
    float frameSpeed;
    int numFrames;
    int damage; // Added: Damage value for the projectile
    ProjectileType type; // Added: Type of projectile (normal, frozen, etc.)

    // Updated Constructor:
    // This constructor matches the arguments you are passing from plant.cpp.
    Projectile(Rectangle pRect, Vector2 pSpeed, int pDamage, Texture2D pTex, ProjectileType pType = ProjectileType::NORMAL)
        : rect(pRect), speed(pSpeed), active(true), damage(pDamage), texture(pTex), type(pType),
          currentFrame(0), frameTimer(0.0f), frameSpeed(0.1f), numFrames(1) // Default animation values
    {
        // Assuming a single-frame texture or a horizontally laid out sprite sheet
        sourceRect = {0, 0, (float)texture.width / numFrames, (float)texture.height};
        // Set color based on type for debugging/fallback if texture not loaded
        if (type == ProjectileType::FROZEN) {
            color = BLUE;
        } else {
            color = LIME;
        }
    }

    // You will need to implement this in projectile.cpp
    // The Update method needs access to Zombies to check for collisions and apply effects.
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies);
    void Draw() const;
};

#endif // PROJECTILE_H