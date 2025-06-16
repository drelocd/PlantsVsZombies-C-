// plant.cpp
#include "plant.h"
#include "projectile.h" // Needed to create Projectile objects
#include "zombie.h"     // Needed to interact with Zombie objects
#include <iostream>     // For debug prints (optional)
#include <algorithm>    // For std::max (CherryBomb)

// Defined global grid constants from main.cpp
// These need to be extern if they are defined in main.cpp and used here.
// Make sure these are defined globally in main.cpp
extern const int GRID_START_X;
extern const int GRID_START_Y;
extern const int TILE_SIZE;
extern const int GRID_ROWS;
extern const int GRID_COLS;
// extern const float SUNFLOWER_SUN_GENERATION_RATE; // If defined in game_constants.h, include that instead

//----------------------------------------------------------------------------------
// Base Plant Implementation
//----------------------------------------------------------------------------------
Plant::Plant(Rectangle rect, int health, Color color, Texture2D tex, int row, int col, int numFrames, float frameSpeed)
    : rect(rect), health(health), active(true), color(color), texture(tex),
      row(row), col(col), // Make sure these match the order in plant.h to avoid -Wreorder
      currentFrame(0), frameTimer(0.0f), frameSpeed(frameSpeed), numFrames(numFrames)
{
    // Initialize sourceRect based on total texture width and number of frames
    sourceRect = {0, 0, (float)texture.width / numFrames, (float)texture.height};
}

void Plant::Draw() const {
    if (active) {
        // Draw the current frame of the plant's animation
        DrawTexturePro(texture, sourceRect, rect, {0, 0}, 0, WHITE);
    }
}

//----------------------------------------------------------------------------------
// Peashooter Implementations
//----------------------------------------------------------------------------------
Peashooter::Peashooter(Rectangle rect, int row, int col, Texture2D tex)
    : Plant(rect, 100, GREEN, tex, row, col, 1, 0.0f), // Assuming 1 frame for peashooter animation, 0 speed
      fireRate(1.5f), fireTimer(1.5f) { // Fires every 1.5 seconds, starts ready
    // If your peashooter.png has multiple frames (e.g., idle animation), adjust numFrames and frameSpeed here
    // Example: Plant(rect, 100, GREEN, tex, row, col, 4, 0.15f) for 4 frames @ 0.15s/frame
}

void Peashooter::Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) {
    if (!active) return;

    // Animation update for Peashooter (if it has one)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        sourceRect.x = currentFrame * sourceRect.width;
    }

    fireTimer += deltaTime;
    if (fireTimer >= fireRate) {
        // Check if there is a zombie in the same row before firing
        bool zombieInLane = false;
        for (const auto& zombie : zombies) {
            if (zombie->active && zombie->row == this->row && zombie->rect.x > this->rect.x) {
                zombieInLane = true;
                break;
            }
        }

        if (zombieInLane) {
            fireTimer = 0.0f; // Reset timer
            // Create a new projectile with the pea texture and NORMAL type
            // Corrected Projectile constructor call
            std::unique_ptr<Projectile> newProjectile = std::make_unique<Projectile>(
                (Rectangle){ this->rect.x + this->rect.width, this->rect.y + this->rect.height / 4, 20, 10 },
                (Vector2){ 300.0f, 0.0f }, // speed
                50,                        // damage
                peaTex,                    // texture
                ProjectileType::NORMAL     // type
            );
            projectiles.push_back(std::move(newProjectile)); // Add to projectiles vector
            PlaySound(shootSound); // Play sound here, passed from main
        }
    }
}

// Draw method for Peashooter (now required)
void Peashooter::Draw() const {
    Plant::Draw(); // Call base class Draw
}

//----------------------------------------------------------------------------------
// Sunflower Implementations
//----------------------------------------------------------------------------------
Sunflower::Sunflower(Rectangle rect, int row, int col, Texture2D tex)
    : Plant(rect, 80, YELLOW, tex, row, col, 1, 0.0f), // Assuming 1 frame for sunflower
      sunProductionInterval(10.0f), sunProductionTimer(0.0f) { // Example: Produces sun every 10 seconds
    // You should define SUNFLOWER_SUN_GENERATION_RATE in game_constants.h if you want to use it
    // For now, I've put a default of 10.0f directly.
    // Adjust numFrames and frameSpeed if you have an animation for sunflower
}

void Sunflower::Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) {
    if (!active) return;

    // Animation update for Sunflower (if it has one)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        sourceRect.x = currentFrame * sourceRect.width;
    }

    sunProductionTimer += deltaTime;
    if (sunProductionTimer >= sunProductionInterval) {
        sunProductionTimer = 0.0f;
        sunCurrency += 25; // Produce 25 sun
        std::cout << "Sunflower produced sun! Current sun: " << sunCurrency << std::endl;
        // Play a sun production sound if you have one (not included in this sound parameter)
    }
}

// Draw method for Sunflower (now required)
void Sunflower::Draw() const {
    Plant::Draw(); // Call base class Draw
}

//----------------------------------------------------------------------------------
// CherryBomb Implementations
//----------------------------------------------------------------------------------
CherryBomb::CherryBomb(Rectangle rect, int row, int col, Texture2D tex, Sound expSound)
    : Plant(rect, 1, RED, tex, row, col, 1, 0.0f), // Very low health, just needs to exist until explosion
      fuseTimer(0.0f), exploded(false), explosionSound(expSound) {
    // Adjust numFrames and frameSpeed if you have an animation for cherry bomb (e.g., blinking fuse)
}

void CherryBomb::Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) {
    if (!active || exploded) return;

    // Animation update for CherryBomb (e.g., for a blinking fuse)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        sourceRect.x = currentFrame * sourceRect.width;
    }

    fuseTimer += deltaTime;
    if (fuseTimer >= FUSE_DURATION) {
        exploded = true;
        this->active = false; // Cherry Bomb deactivates after exploding
        PlaySound(explosionSound); // Play explosion sound

        int explosionDamage = 9999; // High damage to instantly kill most zombies
        // Calculate explosion area (3x3 grid tiles centered on Cherry Bomb)
        Rectangle explosionArea = {
            (float)(GRID_START_X + (this->col - 1) * TILE_SIZE),
            (float)(GRID_START_Y + (this->row - 1) * TILE_SIZE),
            (float)(TILE_SIZE * 3), // 3 tiles wide
            (float)(TILE_SIZE * 3)  // 3 tiles high
        };

        // Clamp explosion area to grid boundaries to prevent out-of-bounds checks
        // Using `std::max` and `std::min` requires `#include <algorithm>`
        explosionArea.x = std::max((float)GRID_START_X, explosionArea.x);
        explosionArea.y = std::max((float)GRID_START_Y, explosionArea.y);
        explosionArea.width = std::min((float)GRID_COLS * TILE_SIZE - (explosionArea.x - GRID_START_X), explosionArea.width);
        explosionArea.height = std::min((float)GRID_ROWS * TILE_SIZE - (explosionArea.y - GRID_START_Y), explosionArea.height);


        for (auto& zombie : zombies) {
            if (zombie->active && CheckCollisionRecs(explosionArea, zombie->rect)) {
                zombie->health -= explosionDamage; // Deal damage
            }
        }
        std::cout << "CherryBomb exploded! Damaged zombies in area." << std::endl;
    }
}

// Draw method for CherryBomb (now required)
void CherryBomb::Draw() const {
    Plant::Draw(); // Call base class Draw
}


//----------------------------------------------------------------------------------
// WallNut Implementations
//----------------------------------------------------------------------------------
WallNut::WallNut(Rectangle rect, int row, int col, Texture2D tex)
    : Plant(rect, 400, BROWN, tex, row, col, 1, 0.0f) { // High health, assumes 1 frame for wall-nut
    // Adjust numFrames and frameSpeed if you have an animation for wall-nut
}

void WallNut::Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) {
    if (!active) return;
    // Animation update for Wall-nut (if it has one)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        sourceRect.x = currentFrame * sourceRect.width;
    }
    // Wall-nuts don't do anything specific in update besides take damage, which is handled by zombie collisions.
}

// Draw method for WallNut (now required)
void WallNut::Draw() const {
    Plant::Draw(); // Call base class Draw
}

//----------------------------------------------------------------------------------
// Repeater Implementations (NEW!)
//----------------------------------------------------------------------------------
Repeater::Repeater(Rectangle rect, int row, int col, Texture2D tex)
    : Peashooter(rect, row, col, tex) // Call base Peashooter constructor
{
    // Repeater's unique properties: fires twice, so faster effective fire rate
    // We achieve this by overriding the Update logic to create two projectiles.
    // The fireRate itself can be similar to Peashooter's, but two projectiles are created.
    // To make it shoot "twice the amount of pea causing twice the damage" per fire event,
    // we can either make it fire two projectiles or make one projectile deal double damage.
    // The prompt implies two separate peas, so we will fire two projectiles.
    this->fireRate = 1.0f; // Slightly faster or same as Peashooter, but produces 2 peas.
    this->fireTimer = this->fireRate; // Start ready to fire
    this->health = 100; // Default health (can be adjusted)
}

void Repeater::Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) {
    if (!active) return;

    // Animation update (similar to Peashooter)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        sourceRect.x = currentFrame * sourceRect.width;
    }

    fireTimer += deltaTime;
    if (fireTimer >= fireRate) {
        bool zombieInLane = false;
        for (const auto& zombie : zombies) {
            if (zombie->active && zombie->row == this->row && zombie->rect.x > this->rect.x) {
                zombieInLane = true;
                break;
            }
        }

        if (zombieInLane) {
            fireTimer = 0.0f; // Reset timer

            // First projectile
            std::unique_ptr<Projectile> newProjectile1 = std::make_unique<Projectile>(
                (Rectangle){ this->rect.x + this->rect.width, this->rect.y + this->rect.height / 4, 20, 10 },
                (Vector2){ 300.0f, 0.0f }, // speed
                50,                        // damage (standard pea damage)
                peaTex,                    // texture
                ProjectileType::NORMAL     // type
            );
            projectiles.push_back(std::move(newProjectile1));
            PlaySound(shootSound);

            // Second projectile (fired immediately after the first for "twice the amount")
            // You can add a small delay here (e.g., 0.1s) for visual effect if desired.
            // For true "twice the amount" per fire *event*, immediate is fine.
            std::unique_ptr<Projectile> newProjectile2 = std::make_unique<Projectile>(
                (Rectangle){ this->rect.x + this->rect.width, this->rect.y + this->rect.height / 4, 20, 10 },
                (Vector2){ 300.0f, 0.0f }, // speed
                50,                        // damage
                peaTex,                    // texture
                ProjectileType::NORMAL     // type
            );
            projectiles.push_back(std::move(newProjectile2));
            // PlaySound(shootSound); // Play sound again if you want a double-shot sound, or just once for the "burst"
        }
    }
}

// Draw method for Repeater (now required, can just call base)
void Repeater::Draw() const {
    Plant::Draw(); // Call base class Draw
}

//----------------------------------------------------------------------------------
// IcePea Implementations (NEW!)
//----------------------------------------------------------------------------------
IcePea::IcePea(Rectangle rect, int row, int col, Texture2D tex, Texture2D icePeaProjTex)
    : Peashooter(rect, row, col, tex), icePeaProjectileTex(icePeaProjTex) // Call base Peashooter constructor
{
    // Ice Pea specific properties
    this->fireRate = 1.8f; // Slightly slower fire rate for slowing effect
    this->fireTimer = this->fireRate; // Start ready to fire
    this->health = 200; // Default health (can be adjusted)
}

void IcePea::Update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies, std::vector<std::unique_ptr<Projectile>>& projectiles, int& sunCurrency, Sound shootSound, Texture2D peaTex) {
    if (!active) return;

    // Animation update (similar to Peashooter)
    frameTimer += deltaTime;
    if (frameTimer >= frameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
        sourceRect.x = currentFrame * sourceRect.width;
    }

    fireTimer += deltaTime;
    if (fireTimer >= fireRate) {
        bool zombieInLane = false;
        for (const auto& zombie : zombies) {
            if (zombie->active && zombie->row == this->row && zombie->rect.x > this->rect.x) {
                zombieInLane = true;
                break;
            }
        }

        if (zombieInLane) {
            fireTimer = 0.0f; // Reset timer

            // Create a FROZEN projectile using the icePeaProjectileTex
            std::unique_ptr<Projectile> newProjectile = std::make_unique<Projectile>(
                (Rectangle){ this->rect.x + this->rect.width, this->rect.y + this->rect.height / 4, 20, 10 },
                (Vector2){ 300.0f, 0.0f }, // speed
                50,                        // damage (same as normal pea)
                icePeaProjectileTex,       // Use the specific ice pea projectile texture
                ProjectileType::FROZEN     // Set type to FROZEN
            );
            projectiles.push_back(std::move(newProjectile));
            PlaySound(shootSound); // You might want a distinct sound for ice peas
        }
    }
}

// Draw method for IcePea (now required, can just call base)
void IcePea::Draw() const {
    Plant::Draw(); // Call base class Draw
}