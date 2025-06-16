#ifndef LAWNMOWER_H
#define LAWNMOWER_H

#include "raylib.h"

class LawnMower {
public:
    Rectangle rect;
    int row;
    Texture2D texture;
    bool active;     // If true, it's currently on screen and potentially moving
    bool activated;  // If true, it has been triggered and is moving across the lane

    float speed;     // Speed at which the lawnmower moves

    LawnMower(Rectangle rect, int row, Texture2D texture);
    void Update(float deltaTime);
    void Draw();
};

#endif // LAWNMOWER_H