#include "raylib.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <string>

// Include headers
#include "game_state.h"
#include "projectile.h"
#include "plant.h"
#include "zombie.h"
#include "game_constants.h"
#include "lawnmower.h"

// Game Constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int GRID_ROWS = 5;
const int GRID_COLS = 9;
const int ORIGINAL_TILE_SIZE = 80;
const int TILE_SIZE = static_cast<int>(ORIGINAL_TILE_SIZE * 1.2f);
const int Y_OFFSET = 80;
const int UI_PANEL_HEIGHT = 120;
const int UI_PANEL_Y = 0;
const int UI_PANEL_PADDING = 10;
const int GRID_START_X = 90;
const int GRID_START_Y = UI_PANEL_HEIGHT + Y_OFFSET;
const int PAUSE_BUTTON_SIZE = 60;
const int PLANT_ICON_SIZE = 70;
const int PLANT_ICON_SPACING = 20;

// Global Variables
int sunCurrency = 50;
PlantType currentSelectedPlantType = PlantType::PEASHOOTER;
int score = 0;
int currentLevel = 1;
int targetScore = 1000;

Music backgroundMusic;
bool isMusicMuted = false;
const float ORIGINAL_MUSIC_VOLUME = 0.5f;
int CalculateTargetScore(int level) {
    return level == 1 ? 1000 : 1000 + (level - 1) * 3000;
}

void ResetGame(std::vector<std::unique_ptr<Plant>>& plants,
              std::vector<std::unique_ptr<Zombie>>& zombies,
              std::vector<std::unique_ptr<Projectile>>& projectiles,
              std::vector<std::unique_ptr<LawnMower>>& lawnmowers,
              Texture2D lawnmowerTex,
              Texture2D regularZombieTex,
              Texture2D jumpingZombieTex,
              float& zombieSpawnTimer,
              float& zombieSpawnRate,
              int& sunCurrency_ref,
              int& score_ref,
              PlantType& currentSelectedPlantType_ref,
              int levelToSet)
{
    plants.clear();
    zombies.clear();
    projectiles.clear();
    lawnmowers.clear();

    for (int i = 0; i < GRID_ROWS; ++i) {
        Rectangle mowerRect = {
            (float)GRID_START_X - TILE_SIZE,
            (float)GRID_START_Y + i * TILE_SIZE,
            TILE_SIZE / 2.0f * 1.8f,
            TILE_SIZE / 2.0f * 1.8f
        };
        lawnmowers.push_back(std::make_unique<LawnMower>(mowerRect, i, lawnmowerTex));
    }

    zombieSpawnTimer = 0.0f;
    sunCurrency_ref = 50;
    score_ref = 0;
    currentSelectedPlantType_ref = PlantType::PEASHOOTER;
    currentLevel = levelToSet;
    targetScore = CalculateTargetScore(currentLevel);

    zombieSpawnRate = 5.0f - (currentLevel - 1) * 0.4f;
    if (zombieSpawnRate < 1.0f) zombieSpawnRate = 1.0f;

    int initialZombies = currentLevel * 2;
    if (initialZombies > 10) initialZombies = 10;

    for (int i = 0; i < initialZombies; ++i) {
        int spawnRow = GetRandomValue(0, GRID_ROWS - 1);
        Rectangle zombieRect = {
            (float)SCREEN_WIDTH + i * TILE_SIZE,
            (float)GRID_START_Y + spawnRow * TILE_SIZE + (TILE_SIZE / 4.0f),
            TILE_SIZE / 2.0f * 2.8f,
            TILE_SIZE / 2.0f * 2.8f
        };
        
        // Fix for the unique_ptr type mismatch error
        std::unique_ptr<Zombie> newZombie;
        if (GetRandomValue(0, 1) == 0) {
            newZombie = std::make_unique<RegularZombie>(zombieRect, spawnRow, regularZombieTex, currentLevel);
        } else {
            newZombie = std::make_unique<JumpingZombie>(zombieRect, spawnRow, jumpingZombieTex, currentLevel);
        }
            
        zombies.push_back(std::move(newZombie));
    }
}

int main(void)
{
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Plants vs. Zombies - C++/Raylib");
    InitAudioDevice();
    backgroundMusic = LoadMusicStream("resources/game_music.mp3");
    SetMusicVolume(backgroundMusic, ORIGINAL_MUSIC_VOLUME);
    PlayMusicStream(backgroundMusic);
    // Load sounds
    Sound shootSound = LoadSound("resources/shoot.mp3");
    Sound hitSound = LoadSound("resources/hit.mp3");
    Sound gameOverSound = LoadSound("resources/gameover.mp3");
    Sound cherryBombExplosionSound = LoadSound("resources/explosion.mp3");
    Sound lawnmowerSound = LoadSound("resources/lawnmower.mp3");
    Sound digSound = LoadSound("resources/dig.mp3");
    

    // Load textures
    Texture2D peashooterTex = LoadTexture("resources/peashooter.png");
    Texture2D sunflowerTex = LoadTexture("resources/sunflower.png");
    Texture2D cherryBombTex = LoadTexture("resources/cherrybomb.png");
    Texture2D wallnutTex = LoadTexture("resources/wallnut.png");
    Texture2D regularZombieTex = LoadTexture("resources/regular_zombie.png");
    Texture2D jumpingZombieTex = LoadTexture("resources/jumping_zombie.png");
    Texture2D peaTex = LoadTexture("resources/pea.png");
    Texture2D grassBackgroundTex = LoadTexture("resources/grass_background.png");
    Texture2D pauseButtonTex = LoadTexture("resources/pause_button.png");
    Texture2D mainMenuBackgroundTex = LoadTexture("resources/main_menu_background.png");
    Texture2D lawnmowerTex = LoadTexture("resources/lawnmower.png");
    Texture2D levelUpTex = LoadTexture("resources/levelup.png");
    Texture2D shovelTex = LoadTexture("resources/shovel.png");
    Texture2D repeaterTex = LoadTexture("resources/repeater.png");
    Texture2D icePeaPlantTex = LoadTexture("resources/icepea.png");
    Texture2D icePeaProjectileTex = LoadTexture("resources/pea.png");
    Texture2D muteIconTex = LoadTexture("resources/mute.png");   
    Texture2D unmuteIconTex = LoadTexture("resources/unmute.png");

    // Define UI rectangles
    Rectangle pauseButtonRect = {
        (float)SCREEN_WIDTH - PAUSE_BUTTON_SIZE - UI_PANEL_PADDING - 40,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PAUSE_BUTTON_SIZE,
        (float)PAUSE_BUTTON_SIZE
    };

    // New mute button rectangle - adjust position as desired!
    Rectangle muteButtonRect = {
        pauseButtonRect.x - PAUSE_BUTTON_SIZE - UI_PANEL_PADDING, // Place it to the left of pause button
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PAUSE_BUTTON_SIZE,
        (float)PAUSE_BUTTON_SIZE
    };
    Rectangle pauseButtonRect = {
        (float)SCREEN_WIDTH - PAUSE_BUTTON_SIZE - UI_PANEL_PADDING - 40,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PAUSE_BUTTON_SIZE,
        (float)PAUSE_BUTTON_SIZE
    };

    Rectangle peashooterIconRect = {
        (float)UI_PANEL_PADDING + 400,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle sunflowerIconRect = {
        peashooterIconRect.x + PLANT_ICON_SIZE + PLANT_ICON_SPACING,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle cherryBombIconRect = {
        sunflowerIconRect.x + PLANT_ICON_SIZE + PLANT_ICON_SPACING,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle wallnutIconRect = {
        cherryBombIconRect.x + PLANT_ICON_SIZE + PLANT_ICON_SPACING,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle shovelIconRect = {
        wallnutIconRect.x + PLANT_ICON_SIZE + PLANT_ICON_SPACING,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle repeaterIconRect = {
        shovelIconRect.x + PLANT_ICON_SIZE + PLANT_ICON_SPACING,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle icePeaIconRect = {
        repeaterIconRect.x + PLANT_ICON_SIZE + PLANT_ICON_SPACING,
        (float)UI_PANEL_Y + UI_PANEL_PADDING,
        (float)PLANT_ICON_SIZE,
        (float)PLANT_ICON_SIZE
    };

    Rectangle continueButtonRect = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 200, 50 };
    Rectangle levelMainMenuButtonRect = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 110, 200, 50 };
    Rectangle replayLevelButtonRect = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 170, 200, 50 };

    // Game objects
    std::vector<std::unique_ptr<Plant>> plants;
    std::vector<std::unique_ptr<Zombie>> zombies;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<LawnMower>> lawnmowers;

    // Game state
    float zombieSpawnTimer = 0.0f;
    float zombieSpawnRate = 5.0f;
    GameState currentGameState = MAIN_MENU;

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateMusicStream(backgroundMusic);
        switch (currentGameState) {
            case MAIN_MENU: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mousePos = GetMousePosition();
                    Rectangle playButton = { 250, SCREEN_HEIGHT / 2 - 80, 300, 100 };
                    Rectangle exitButton = { 950, SCREEN_HEIGHT / 2 + 250, 300, 100 };

                    if (CheckCollisionPointRec(mousePos, playButton)) {
                        ResetGame(plants, zombies, projectiles, lawnmowers, lawnmowerTex,
                                  regularZombieTex, jumpingZombieTex,
                                  zombieSpawnTimer, zombieSpawnRate, sunCurrency, score,
                                  currentSelectedPlantType, 1);
                        currentGameState = GAMEPLAY;
                    } else if (CheckCollisionPointRec(mousePos, exitButton)) {
                        CloseWindow();
                    }
                }
                break;
            }

            case GAMEPLAY: {
                case GAMEPLAY: {
                    // Input handling
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 mousePos = GetMousePosition();
    
                        if (CheckCollisionPointRec(mousePos, pauseButtonRect)) {
                            currentGameState = PAUSED;
                        }
                        // --- NEW MUTE BUTTON LOGIC START ---
                        else if (CheckCollisionPointRec(mousePos, muteButtonRect)) {
                            isMusicMuted = !isMusicMuted; // Toggle the mute state
                            if (isMusicMuted) {
                                SetMusicVolume(backgroundMusic, 0.0f); // Mute the music
                            } else {
                                SetMusicVolume(backgroundMusic, ORIGINAL_MUSIC_VOLUME); // Unmute to original volume
                            }
                        }
                        // --- NEW MUTE BUTTON LOGIC END ---
                        else if (CheckCollisionPointRec(mousePos, peashooterIconRect)) {
                            if (sunCurrency >= 50) currentSelectedPlantType = PlantType::PEASHOOTER;
                            else std::cout << "Not enough sun for Peashooter!" << std::endl;
                        }
                        else if (CheckCollisionPointRec(mousePos, sunflowerIconRect)) {
                            if (sunCurrency >= 25) currentSelectedPlantType = PlantType::SUNFLOWER;
                            else std::cout << "Not enough sun for Sunflower!" << std::endl;
                        }
                        else if (CheckCollisionPointRec(mousePos, cherryBombIconRect)) {
                            if (sunCurrency >= 50) currentSelectedPlantType = PlantType::CHERRY_BOMB;
                            else std::cout << "Not enough sun for Cherry Bomb!" << std::endl;
                        }
                        else if (CheckCollisionPointRec(mousePos, wallnutIconRect)) {
                            if (sunCurrency >= 75) currentSelectedPlantType = PlantType::WALNUT;
                            else std::cout << "Not enough sun for Wall-nut!" << std::endl;
                        }
                        else if (CheckCollisionPointRec(mousePos, shovelIconRect)) {
                            currentSelectedPlantType = PlantType::SHOVEL;
                        }
                        else if (CheckCollisionPointRec(mousePos, repeaterIconRect)) {
                            if (sunCurrency >= 200) currentSelectedPlantType = PlantType::REPEATER;
                            else std::cout << "Not enough sun for Repeater!" << std::endl;
                        }
                        else if (CheckCollisionPointRec(mousePos, icePeaIconRect)) {
                            if (sunCurrency >= 150) currentSelectedPlantType = PlantType::ICE_PEA;
                            else std::cout << "Not enough sun for Ice Pea!" << std::endl;
                        }
                        else if (CheckCollisionPointRec(mousePos, { (float)GRID_START_X, (float)GRID_START_Y,
                                                (float)(GRID_COLS * TILE_SIZE), (float)(GRID_ROWS * TILE_SIZE) })) {
                            int col = (mousePos.x - GRID_START_X) / TILE_SIZE;
                            int row = (mousePos.y - GRID_START_Y) / TILE_SIZE;
    
                            if (currentSelectedPlantType == PlantType::SHOVEL) {
                                for (int i = plants.size() - 1; i >= 0; --i) {
                                    if (plants[i]->row == row && plants[i]->col == col) {
                                        plants.erase(plants.begin() + i);
                                        PlaySound(digSound); // Make sure digSound is loaded correctly
                                        break;
                                    }
                                }
                            } else {
                                bool plantExists = false;
                                for (const auto& plant : plants) {
                                    if (plant->row == row && plant->col == col) {
                                        plantExists = true;
                                        break;
                                    }
                                }
    
                                if (!plantExists) {
                                    std::unique_ptr<Plant> newPlant = nullptr;
                                    Rectangle plantRect = {
                                        (float)GRID_START_X + col * TILE_SIZE + (TILE_SIZE / 4.0f),
                                        (float)GRID_START_Y + row * TILE_SIZE + (TILE_SIZE / 4.0f),
                                        TILE_SIZE / 2.0f * 1.8f,
                                        TILE_SIZE / 2.0f * 1.8f
                                    };
    
                                    switch (currentSelectedPlantType) {
                                        case PlantType::PEASHOOTER:
                                            if (sunCurrency >= 50) newPlant = std::make_unique<Peashooter>(plantRect, row, col, peashooterTex);
                                            break;
                                        case PlantType::SUNFLOWER:
                                            if (sunCurrency >= 25) newPlant = std::make_unique<Sunflower>(plantRect, row, col, sunflowerTex);
                                            break;
                                        case PlantType::CHERRY_BOMB:
                                            if (sunCurrency >= 50) newPlant = std::make_unique<CherryBomb>(plantRect, row, col, cherryBombTex, cherryBombExplosionSound);
                                            break;
                                        case PlantType::WALNUT:
                                            if (sunCurrency >= 75) newPlant = std::make_unique<WallNut>(plantRect, row, col, wallnutTex);
                                            break;
                                        case PlantType::REPEATER:
                                            if (sunCurrency >= 200) newPlant = std::make_unique<Repeater>(plantRect, row, col, repeaterTex);
                                            break;
                                        case PlantType::ICE_PEA:
                                            if (sunCurrency >= 150) newPlant = std::make_unique<IcePea>(plantRect, row, col, icePeaPlantTex, icePeaProjectileTex);
                                            break;
                                        default:
                                            break;
                                    }
    
                                    if (newPlant) {
                                        sunCurrency -= newPlant->GetCost();
                                        plants.push_back(std::move(newPlant));
                                    }
                                }
                            }
                        }
                    }
    
                    // Level completion check
                    if (score >= targetScore) {
                        currentGameState = LEVEL_UP_SCREEN;
                        break;
                    }
                // Zombie spawning
                zombieSpawnTimer += deltaTime;
                if (zombieSpawnTimer >= zombieSpawnRate) {
                    zombieSpawnTimer = 0.0f;
                    int spawnRow = GetRandomValue(0, GRID_ROWS - 1);

                    std::unique_ptr<Zombie> newZombie = nullptr;
                    Rectangle zombieRect = {
                        (float)SCREEN_WIDTH,
                        (float)GRID_START_Y + spawnRow * TILE_SIZE + (TILE_SIZE / 4.0f),
                        TILE_SIZE / 2.0f * 2.8f,
                        TILE_SIZE / 2.0f * 2.8f
                    };

                    // Fix for the unique_ptr type mismatch error
                    if (GetRandomValue(0, 1) == 0) {
                        newZombie = std::make_unique<RegularZombie>(zombieRect, spawnRow, regularZombieTex, currentLevel);
                    } else {
                        newZombie = std::make_unique<JumpingZombie>(zombieRect, spawnRow, jumpingZombieTex, currentLevel);
                    }

                    zombies.push_back(std::move(newZombie));
                }

                // Update plants
                for (auto& plant : plants) {
                    if (plant->active) {
                        plant->Update(deltaTime, zombies, projectiles, sunCurrency, shootSound, peaTex);
                    }
                }

                // Update zombies
                for (int i = zombies.size() - 1; i >= 0; --i) {
                    zombies[i]->Update(deltaTime, plants);

                    if (zombies[i]->health <= 0 && zombies[i]->active) {
                        score += zombies[i]->scoreValue;
                        zombies[i]->active = false;
                    }

                    if (!zombies[i]->active) {
                        zombies.erase(zombies.begin() + i);
                        continue;
                    }

                    if (zombies[i]->rect.x <= GRID_START_X - TILE_SIZE / 2 && (size_t)zombies[i]->row < lawnmowers.size()) {
                        LawnMower* mower = lawnmowers[zombies[i]->row].get();
                        if (mower && !mower->activated) {
                            mower->activated = true;
                            PlaySound(lawnmowerSound);
                        }
                    }

                    if (zombies[i]->rect.x < GRID_START_X - TILE_SIZE) {
                        currentGameState = GAME_OVER;
                        PlaySound(gameOverSound);
                        break;
                    }
                }

                if (currentGameState == GAME_OVER) break;

                // Update projectiles
                for (int p_idx = projectiles.size() - 1; p_idx >= 0; --p_idx) {
                    projectiles[p_idx]->rect.x += projectiles[p_idx]->speed.x * deltaTime;

                    if (projectiles[p_idx]->rect.x > SCREEN_WIDTH) {
                        projectiles[p_idx]->active = false;
                    }
                    
                    if (!projectiles[p_idx]->active) {
                        projectiles.erase(projectiles.begin() + p_idx);
                        continue;
                    }
                    
                    for (auto& zombie : zombies) {
                        if (!zombie->active) continue;

                        if (CheckCollisionRecs(projectiles[p_idx]->rect, zombie->rect)) {
                            // Apply projectile effects based on type
                            if (projectiles[p_idx]->type == ProjectileType::FROZEN) {
                                // Slow down zombie, assuming Zombie class has a method for this
                                zombie->ApplySlowEffect(); 
                            }
                            zombie->health -= projectiles[p_idx]->damage; // Use projectile's damage value
                            projectiles[p_idx]->active = false; // Deactivate projectile after hit
                            PlaySound(hitSound);

                            if (zombie->health <= 0) { // Only add score if zombie is actually defeated by this projectile
                                score += zombie->scoreValue;
                                zombie->active = false; // Mark zombie as inactive
                            }
                            break; // Projectile hit one zombie, so it's done
                        }
                    }
                }

                // Update lawnmowers
                for (auto& mower : lawnmowers) {
                    if (mower->activated && mower->active) {
                        mower->Update(deltaTime);
                        for (auto& zombie : zombies) {
                            if (zombie->active && mower->row == zombie->row && CheckCollisionRecs(mower->rect, zombie->rect)) {
                                if (zombie->active) { // Check active again in case it was just deactivated by another mower in the same frame (unlikely but safe)
                                    score += zombie->scoreValue;
                                }
                                zombie->health = 0; // Instantly kill zombie
                                zombie->active = false;
                            }
                        }
                        if (mower->rect.x > SCREEN_WIDTH + TILE_SIZE) {
                            mower->active = false;
                        }
                    }
                }

                // Cleanup inactive plants
                plants.erase(std::remove_if(plants.begin(), plants.end(), 
                                [](const std::unique_ptr<Plant>& p){ return !p->active; }), 
                                plants.end());

                break;
            }

            case PAUSED: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mousePos = GetMousePosition();
                    Rectangle resumeButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 50 };
                    Rectangle exitButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20, 200, 50 };

                    if (CheckCollisionPointRec(mousePos, resumeButton)) {
                        currentGameState = GAMEPLAY;
                    } else if (CheckCollisionPointRec(mousePos, exitButton)) {
                        ResetGame(plants, zombies, projectiles, lawnmowers, lawnmowerTex,
                                  regularZombieTex, jumpingZombieTex,
                                  zombieSpawnTimer, zombieSpawnRate, sunCurrency, score,
                                  currentSelectedPlantType, 1);
                        currentGameState = MAIN_MENU;
                    }
                }
                break;
            }

            case LEVEL_UP_SCREEN: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mousePos = GetMousePosition();
                    if (CheckCollisionPointRec(mousePos, continueButtonRect)) {
                        ResetGame(plants, zombies, projectiles, lawnmowers, lawnmowerTex,
                                  regularZombieTex, jumpingZombieTex,
                                  zombieSpawnTimer, zombieSpawnRate, sunCurrency, score,
                                  currentSelectedPlantType, currentLevel + 1);
                        currentGameState = GAMEPLAY;
                    } else if (CheckCollisionPointRec(mousePos, levelMainMenuButtonRect)) {
                        ResetGame(plants, zombies, projectiles, lawnmowers, lawnmowerTex,
                                  regularZombieTex, jumpingZombieTex,
                                  zombieSpawnTimer, zombieSpawnRate, sunCurrency, score,
                                  currentSelectedPlantType, 1);
                        currentGameState = MAIN_MENU;
                    } else if (CheckCollisionPointRec(mousePos, replayLevelButtonRect)) {
                        ResetGame(plants, zombies, projectiles, lawnmowers, lawnmowerTex,
                                  regularZombieTex, jumpingZombieTex,
                                  zombieSpawnTimer, zombieSpawnRate, sunCurrency, score,
                                  currentSelectedPlantType, currentLevel);
                        currentGameState = GAMEPLAY;
                    }
                }
                break;
            }

            case GAME_OVER: {
                if (IsKeyPressed(KEY_R)) {
                    ResetGame(plants, zombies, projectiles, lawnmowers, lawnmowerTex,
                            regularZombieTex, jumpingZombieTex,
                            zombieSpawnTimer, zombieSpawnRate, sunCurrency, score,
                            currentSelectedPlantType, 1);
                    currentGameState = GAMEPLAY;
                }
                if (IsKeyPressed(KEY_Q)) {
                    CloseWindow();
                }
                break;
            }
        }

        // Drawing
        BeginDrawing();
            ClearBackground(DARKGRAY);

            if (currentGameState == MAIN_MENU) {
                DrawTexturePro(mainMenuBackgroundTex,
                              (Rectangle){ 0, 0, (float)mainMenuBackgroundTex.width, (float)mainMenuBackgroundTex.height },
                              (Rectangle){ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
                              (Vector2){ 0, 0 }, 0.0f, WHITE);

                Rectangle playButton = { 250, SCREEN_HEIGHT / 2 - 80, 300, 100 };
                DrawRectangleRec(playButton, BLANK);
                DrawText("PLAY", playButton.x + (playButton.width - MeasureText("PLAY", 40)) / 2, 
                                 playButton.y + (playButton.height - 40) / 2, 40, BLANK);

                Rectangle exitButton = { 950, SCREEN_HEIGHT / 2 + 250, 300, 100 };
                DrawRectangleRec(exitButton, RED);
                DrawText("EXIT", exitButton.x + (exitButton.width - MeasureText("EXIT", 40)) / 2, 
                                 exitButton.y + (exitButton.height - 40) / 2, 40, BLACK);
            }
            else if (currentGameState == LEVEL_UP_SCREEN) {
                DrawTexturePro(grassBackgroundTex,
                              (Rectangle){ 0, 0, (float)grassBackgroundTex.width, (float)grassBackgroundTex.height },
                              (Rectangle){ (float)GRID_START_X, (float)GRID_START_Y, 
                                           (float)(GRID_COLS * TILE_SIZE), (float)(GRID_ROWS * TILE_SIZE) },
                              (Vector2){ 0, 0 }, 0.0f, WHITE);

                DrawTexturePro(levelUpTex,
                              (Rectangle){ 0, 0, (float)levelUpTex.width, (float)levelUpTex.height },
                              (Rectangle){ SCREEN_WIDTH / 2 - levelUpTex.width / 2.0f, 
                                           SCREEN_HEIGHT / 2 - levelUpTex.height / 2.0f - 100, 
                                           (float)levelUpTex.width, (float)levelUpTex.height },
                              (Vector2){ 0, 0 }, 0.0f, WHITE);

                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));

                std::string levelUpText = "LEVEL " + std::to_string(currentLevel) + " COMPLETE!";
                DrawText(levelUpText.c_str(), 
                                 SCREEN_WIDTH / 2 - MeasureText(levelUpText.c_str(), 60) / 2, 
                                 SCREEN_HEIGHT / 2 - 120, 60, YELLOW);

                std::string nextTargetText = "Next Target: " + std::to_string(CalculateTargetScore(currentLevel + 1)) + " Points";
                DrawText(nextTargetText.c_str(), 
                                 SCREEN_WIDTH / 2 - MeasureText(nextTargetText.c_str(), 30) / 2, 
                                 SCREEN_HEIGHT / 2 - 50, 30, RAYWHITE);

                DrawRectangleRec(continueButtonRect, GREEN);
                DrawText("CONTINUE", 
                                 continueButtonRect.x + (continueButtonRect.width - MeasureText("CONTINUE", 30)) / 2, 
                                 continueButtonRect.y + (continueButtonRect.height - 30) / 2, 30, BLACK);

                DrawRectangleRec(levelMainMenuButtonRect, GRAY);
                DrawText("MAIN MENU", 
                                 levelMainMenuButtonRect.x + (levelMainMenuButtonRect.width - MeasureText("MAIN MENU", 30)) / 2, 
                                 levelMainMenuButtonRect.y + (levelMainMenuButtonRect.height - 30) / 2, 30, BLACK);

                DrawRectangleRec(replayLevelButtonRect, BLUE);
                DrawText("REPLAY", 
                                 replayLevelButtonRect.x + (replayLevelButtonRect.width - MeasureText("REPLAY", 30)) / 2, 
                                 replayLevelButtonRect.y + (replayLevelButtonRect.height - 30) / 2, 30, BLACK);
            }
            else {
                // Draw UI panel
                DrawRectangle(0, UI_PANEL_Y, SCREEN_WIDTH, UI_PANEL_HEIGHT, CLITERAL(Color){ 50, 50, 50, 255 });

                // Draw lawn background
                DrawTexturePro(grassBackgroundTex,
                              (Rectangle){ 0, 0, (float)grassBackgroundTex.width, (float)grassBackgroundTex.height },
                              (Rectangle){ (float)GRID_START_X, (float)GRID_START_Y, 
                                           (float)(GRID_COLS * TILE_SIZE), (float)(GRID_ROWS * TILE_SIZE) },
                              (Vector2){ 0, 0 }, 0.0f, WHITE);

                // Draw game objects
                if (currentGameState == GAMEPLAY) {
                    for (const auto& plant : plants) {
                        plant->Draw();
                    }
                    for (const auto& zombie : zombies) {
                        zombie->Draw();
                    }
                    for (const auto& projectile : projectiles) {
                        if (projectile->active) {
                            DrawTextureRec(projectile->texture, projectile->sourceRect, 
                                           {projectile->rect.x, projectile->rect.y}, WHITE);
                        }
                    }
                    for (const auto& mower : lawnmowers) {
                        mower->Draw();
                    }

                    // Draw UI elements
                    std::string sunText = "Sun: $" + std::to_string(sunCurrency);
                    DrawText(sunText.c_str(), UI_PANEL_PADDING, UI_PANEL_Y + UI_PANEL_PADDING, 20, YELLOW);

                    std::string scoreText = "Score: " + std::to_string(score);
                    DrawText(scoreText.c_str(), UI_PANEL_PADDING, UI_PANEL_Y + UI_PANEL_PADDING + 25, 20, WHITE);

                    std::string levelInfoText = "Level: " + std::to_string(currentLevel) + " | Target: " + std::to_string(targetScore);
                    DrawText(levelInfoText.c_str(), UI_PANEL_PADDING, UI_PANEL_Y + UI_PANEL_PADDING + 50, 20, RAYWHITE);

                    // Draw plant selection icons
                    DrawTextureEx(peashooterTex, (Vector2){peashooterIconRect.x, peashooterIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)peashooterTex.width, WHITE);
                    DrawText("$50", peashooterIconRect.x, peashooterIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::PEASHOOTER) {
                        DrawRectangleLinesEx(peashooterIconRect, 3, YELLOW);
                    }

                    DrawTextureEx(sunflowerTex, (Vector2){sunflowerIconRect.x, sunflowerIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)sunflowerTex.width, WHITE);
                    DrawText("$25", sunflowerIconRect.x, sunflowerIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::SUNFLOWER) {
                        DrawRectangleLinesEx(sunflowerIconRect, 3, YELLOW);
                    }

                    DrawTextureEx(cherryBombTex, (Vector2){cherryBombIconRect.x, cherryBombIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)cherryBombTex.width, WHITE);
                    DrawText("$50", cherryBombIconRect.x, cherryBombIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::CHERRY_BOMB) {
                        DrawRectangleLinesEx(cherryBombIconRect, 3, YELLOW);
                    }

                    DrawTextureEx(wallnutTex, (Vector2){wallnutIconRect.x, wallnutIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)wallnutTex.width, WHITE);
                    DrawText("$75", wallnutIconRect.x, wallnutIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::WALNUT) {
                        DrawRectangleLinesEx(wallnutIconRect, 3, YELLOW);
                    }

                    DrawTextureEx(shovelTex, (Vector2){shovelIconRect.x, shovelIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)shovelTex.width, WHITE);
                    DrawText("$0", shovelIconRect.x, shovelIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::SHOVEL) {
                        DrawRectangleLinesEx(shovelIconRect, 3, YELLOW);
                    }

                    DrawTextureEx(repeaterTex, (Vector2){repeaterIconRect.x, repeaterIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)repeaterTex.width, WHITE);
                    DrawText("$200", repeaterIconRect.x, repeaterIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::REPEATER) {
                        DrawRectangleLinesEx(repeaterIconRect, 3, YELLOW);
                    }

                    DrawTextureEx(icePeaPlantTex, (Vector2){icePeaIconRect.x, icePeaIconRect.y}, 
                                  0.0f, PLANT_ICON_SIZE / (float)icePeaPlantTex.width, WHITE);
                    DrawText("$150", icePeaIconRect.x, icePeaIconRect.y + PLANT_ICON_SIZE + 5, 15, WHITE);
                    if (currentSelectedPlantType == PlantType::ICE_PEA) {
                        DrawRectangleLinesEx(icePeaIconRect, 3, YELLOW);
                    }

                    // Draw pause button
                    DrawTextureRec(pauseButtonTex,
                                   (Rectangle){ 0, 0, (float)pauseButtonTex.width, (float)pauseButtonTex.height },
                                   (Vector2){ pauseButtonRect.x, pauseButtonRect.y },
                                   WHITE);
                }

                // Draw GAME OVER screen
if (currentGameState == GAME_OVER) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
    DrawText("GAME OVER!", SCREEN_WIDTH / 2 - MeasureText("GAME OVER!", 80) / 2,
                             SCREEN_HEIGHT / 2 - 80, 80, RED);

    // --- Adjustments for "Your Score:" and actual score ---
    const char* scoreLabel = "Your Score: ";
    std::string scoreValueStr = std::to_string(score); // Convert score to string once

    int scoreLabelWidth = MeasureText(scoreLabel, 40);
    int scoreValueWidth = MeasureText(scoreValueStr.c_str(), 40);

    // Desired gap between the label and the score value
    int horizontalGap = 10; // Adjust this value to change the space between "Your Score:" and the number

    // Calculate the total width of the combined text block (label + gap + value)
    int combinedWidth = scoreLabelWidth + horizontalGap + scoreValueWidth;

    // Calculate the starting X position for the combined block to be centered
    int startX = SCREEN_WIDTH / 2 - combinedWidth / 2;
    int scoreY = SCREEN_HEIGHT / 2 + 10; // Y-position remains the same

    // Draw "Your Score:" text
    DrawText(scoreLabel, startX, scoreY, 40, WHITE);

    // Draw the actual score value, positioned after the label with the desired gap
    DrawText(scoreValueStr.c_str(), startX + scoreLabelWidth + horizontalGap, scoreY, 40, YELLOW);
    // --- End of adjustments ---

    DrawText("Press 'R' to Restart or 'Q' to Quit",
                             SCREEN_WIDTH / 2 - MeasureText("Press 'R' to Restart or 'Q' to Quit", 30) / 2,
                             SCREEN_HEIGHT / 2 + 80, 30, WHITE);
}
                // Draw PAUSED screen
                if (currentGameState == PAUSED) {
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
                    DrawText("PAUSED", SCREEN_WIDTH / 2 - MeasureText("PAUSED", 80) / 2, 
                                 SCREEN_HEIGHT / 2 - 150, 80, RAYWHITE);

                    Rectangle resumeButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 50 };
                    DrawRectangleRec(resumeButton, LIGHTGRAY);
                    DrawText("RESUME", resumeButton.x + (resumeButton.width - MeasureText("RESUME", 30)) / 2, resumeButton.y + (resumeButton.height - 30) / 2, 30, BLACK);

                    Rectangle exitButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20, 200, 50 };
                    DrawRectangleRec(exitButton, GRAY);
                    DrawText("MAIN MENU", exitButton.x + (exitButton.width - MeasureText("MAIN MENU", 30)) / 2, exitButton.y + (exitButton.height - 30) / 2, 30, BLACK);
                }
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload all loaded sounds
    UnloadSound(shootSound);
    UnloadSound(hitSound);
    UnloadSound(gameOverSound);
    UnloadSound(cherryBombExplosionSound);
    UnloadSound(lawnmowerSound);
    UnloadSound(digSound); 
    UnloadMusicStream(backgroundMusic);

    // Unload all loaded textures
    UnloadTexture(peashooterTex);
    UnloadTexture(sunflowerTex);
    UnloadTexture(cherryBombTex);
    UnloadTexture(wallnutTex);
    UnloadTexture(regularZombieTex);
    UnloadTexture(jumpingZombieTex);
    UnloadTexture(peaTex);
    UnloadTexture(grassBackgroundTex);
    UnloadTexture(pauseButtonTex);
    UnloadTexture(mainMenuBackgroundTex);
    UnloadTexture(lawnmowerTex);
    UnloadTexture(levelUpTex); 
    UnloadTexture(shovelTex); 
    UnloadTexture(repeaterTex);  // Unload repeater texture
    UnloadTexture(icePeaPlantTex); // Unload ice pea plant texture
    UnloadTexture(icePeaProjectileTex); // Unload ice pea projectile texture

    CloseAudioDevice();
    CloseWindow();

    return 0;
}