// game_constants.h
#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

//----------------------------------------------------------------------------------
// Global Screen and Grid Constants
// Defined as extern because their actual values are typically set in main.cpp
// or another single .cpp file to avoid multiple definitions.
//----------------------------------------------------------------------------------
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int GRID_ROWS;
extern const int GRID_COLS;
extern const int TILE_SIZE; // Assumes a square tile size
extern const int GRID_START_X; // X-coordinate where the grid starts
extern const int GRID_START_Y; // Y-coordinate where the grid starts

//----------------------------------------------------------------------------------
// Game-wide Behavior Constants
//----------------------------------------------------------------------------------
// Cherry Bomb specific: time until it explodes after placement
const float FUSE_DURATION = 1.5f; // Seconds

//----------------------------------------------------------------------------------
// Zombie Animation and Behavior Constants
// IMPORTANT: Adjust these values to match your actual sprite sheets and desired game balance.
//----------------------------------------------------------------------------------

// Base Zombie Stats (can be overridden by derived classes or scaled by level)
// These are default values if not specified otherwise for a derived type.
const int ZOMBIE_HEALTH = 100;              // Base health for a generic zombie
const float ZOMBIE_SPEED = 20.0f;           // Base movement speed (pixels per second)
const float ZOMBIE_DAMAGE_PER_SECOND = 10.0f; // Damage plants take per second (alternative to bite system if used)

// Zombie Attack/Bite System specific constants
const int ZOMBIE_DAMAGE_PER_BITE = 20;      // Damage dealt per single bite by a zombie
const float ZOMBIE_BITE_RATE = 0.8f;        // Seconds between each bite action

// Regular Zombie specific stats and animation properties
const int REGULAR_ZOMBIE_HEALTH = 100;      // Health specific to RegularZombie
const float REGULAR_ZOMBIE_SPEED = 20.0f;   // Speed specific to RegularZombie

const int REGULAR_ZOMBIE_TOTAL_SPRITE_ROWS = 2; // Total rows in the regular zombie sprite sheet (e.g., walking, eating)
const int REGULAR_ZOMBIE_WALKING_NUM_FRAMES = 10;    // Number of frames for walking animation (e.g., in row 0)
const float REGULAR_ZOMBIE_WALKING_FRAME_SPEED = 0.25f; // Speed of walking animation (seconds per frame)
const int REGULAR_ZOMBIE_EATING_NUM_FRAMES = 10;     // Number of frames for eating animation (e.g., in row 1)
const float REGULAR_ZOMBIE_EATING_FRAME_SPEED = 0.15f; // Speed of eating animation

// Jumping Zombie specific stats and animation properties
const int JUMPING_ZOMBIE_HEALTH = 150;      // Jumping zombies are often tougher
const float JUMPING_ZOMBIE_SPEED = 25.0f;   // May be slightly faster to make jumps more challenging

const int JUMPING_ZOMBIE_NUM_FRAMES = 6;    // Number of frames for jumping zombie animation
const float JUMPING_ZOMBIE_FRAME_SPEED = 0.15f; // Animation speed for jumping zombie
const int JUMPING_ZOMBIE_TOTAL_SPRITE_ROWS = 1; // Assuming one row for jumping zombie animation

// Zombie Score Values (points awarded when a zombie is defeated)
const int REGULAR_ZOMBIE_SCORE_VALUE = 100;
const int JUMPING_ZOMBIE_SCORE_VALUE = 200;

//----------------------------------------------------------------------------------
// Plant Constants
// Define constants for different plant types here.
//----------------------------------------------------------------------------------

// Peashooter
const int PEASHOOTER_HEALTH = 100;
const int PEASHOOTER_DAMAGE = 20;

// Sunflower
const int SUNFLOWER_HEALTH = 80;
const float SUNFLOWER_SUN_GENERATION_INTERVAL = 0.1f; // Seconds between sun generation

// Add other plant types (Wall-nut, Cherry Bomb, Snow Pea, etc.) as needed
// Example:
// const int WALNUT_HEALTH = 500; // High health
// const int CHERRY_BOMB_HEALTH = 1; // Low health, explodes on contact/timer
// const int CHERRY_BOMB_EXPLOSION_RADIUS = 2; // Grid tiles radius

#endif // GAME_CONSTANTS_H