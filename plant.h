// plant.h
#ifndef PLANT_H
#define PLANT_H

#include "raylib.h"
#include <vector> // Required for interaction with zombie/projectile vectors
#include <memory> // Required for std::unique_ptr


// Forward declarations to avoid circular dependencies
// These are needed because Plant methods might interact with Zombies or Projectiles
// IMPORTANT: These should be 'class' if they are classes, not 'struct' unless they are POD structs.
class Zombie;
class Projectile;


// Enum to identify different plant types
enum class PlantType {
    PEASHOOTER,
    SUNFLOWER,
    CHERRY_BOMB,
    WALNUT,
    SHOVEL,
    REPEATER,
    ICE_PEA,
    NONE // Default or unselected type
};

//----------------------------------------------------------------------------------
// Base Plant Class
//----------------------------------------------------------------------------------
class Plant {
public:
    Rectangle rect;
    int health;
    bool active;
    Color color; // Fallback or for debugging colors, mostly overridden by texture
    Texture2D texture; // The sprite/texture for the plant
    Rectangle sourceRect; // Current frame in the sprite sheet

    // Fix for -Wreorder warning: Reorder members to match constructor initialization order
    int row;
    int col; // Added column for more precise grid placement knowledge

    int currentFrame;
    float frameTimer;
    float frameSpeed;
    int numFrames;

    Plant(Rectangle rect, int health, Color color, Texture2D tex, int row, int col, int numFrames, float frameSpeed);
    virtual ~Plant() = default; // Virtual destructor for proper cleanup of derived objects

    // Pure virtual functions - must be implemented by derived classes
    virtual void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) = 0;
    virtual void Draw() const; // Default draw using texture - marked const, and now virtual (was missing `virtual`)
    virtual int GetCost() const = 0;
    virtual PlantType GetType() const = 0;

    // Common plant methods (can be overridden but not necessarily pure virtual)
    void TakeDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            active = false;
        }
    }

protected: // Changed from private to protected so derived classes can access
    // These should actually be in Peashooter or its own base class if common to all shooting plants
    // but for now, moving them to protected in Plant allows Repeater/IcePea to access them via Peashooter inheritance.
    // Ideally, Peashooter would have these, and Repeater/IcePea inherit them.
    // If Peashooter has them private, then Repeater/IcePea cannot access them.
    // Let's move them into Peashooter as protected, as that's where they logically belong.
    // (Removed from base Plant, will put in Peashooter below)
};

//----------------------------------------------------------------------------------
// Derived Plant Classes
//----------------------------------------------------------------------------------

// Peashooter
class Peashooter : public Plant {
protected: // Changed from private to protected for derived classes (Repeater, IcePea) to access
    float fireRate;
    float fireTimer;

public:
    Peashooter(Rectangle rect, int row, int col, Texture2D tex);
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) override;
    void Draw() const override; // Mark as const to match base
    int GetCost() const override { return 50; } // Cost for Peashooter
    PlantType GetType() const override { return PlantType::PEASHOOTER; }
};

// Sunflower
class Sunflower : public Plant {
private:
    float sunProductionInterval; // Time between sun production
    float sunProductionTimer;    // Timer to track sun production

public:
    Sunflower(Rectangle rect, int row, int col, Texture2D tex);
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) override;
    void Draw() const override; // Mark as const to match base
    int GetCost() const override { return 25; } // Cost for Sunflower
    PlantType GetType() const override { return PlantType::SUNFLOWER; }
};

// CherryBomb
class CherryBomb : public Plant {
private:
    float fuseTimer; // Time until explosion
    // const float FUSE_DURATION = 1.5f; // Member initializers can only be used for non-static data members
                                      // If this is meant to be a constant for all CherryBombs, make it static const or use a #define
                                      // For now, removing it here, assume it's extern or defined in .cpp
    bool exploded;
    Sound explosionSound; // CherryBomb has its own explosion sound

public:
    CherryBomb(Rectangle rect, int row, int col, Texture2D tex, Sound expSound);
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) override;
    void Draw() const override; // Mark as const to match base
    int GetCost() const override { return 50; } // Cost for Cherry Bomb
    PlantType GetType() const override { return PlantType::CHERRY_BOMB; }
};

// WallNut
class WallNut : public Plant {
public:
    WallNut(Rectangle rect, int row, int col, Texture2D tex);
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) override;
    void Draw() const override; // Mark as const to match base
    int GetCost() const override { return 75; } // Cost for Wall-nut
    PlantType GetType() const override { return PlantType::WALNUT; }
};

// --- NEW Plant Classes ---

// Repeater
class Repeater : public Peashooter { // Repeater can inherit from Peashooter as it's similar
public:
    Repeater(Rectangle rect, int row, int col, Texture2D tex);
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) override;
    void Draw() const override; // <-- ADDED: Matches implementation in plant.cpp
    int GetCost() const override { return 200; }
    PlantType GetType() const override { return PlantType::REPEATER; }
};

// IcePea
class IcePea : public Peashooter { // IcePea can also inherit from Peashooter
private:
    Texture2D icePeaProjectileTex; // Specific texture for the ice pea projectile
public:
    IcePea(Rectangle rect, int row, int col, Texture2D tex, Texture2D icePeaProjTex); // Constructor takes projectile texture
    void Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) override;
    void Draw() const override; // <-- ADDED: Matches implementation in plant.cpp
    int GetCost() const override { return 150; }
    PlantType GetType() const override { return PlantType::ICE_PEA; }
};

#endif // PLANT_H