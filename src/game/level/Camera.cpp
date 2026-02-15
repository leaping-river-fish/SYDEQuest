#include "Camera.h"
#include "game/player/Player.h"
#include "Level.h"
#include <algorithm>

Camera::Camera(int screenWidth, int screenHeight)
    : position(0, 0),
      screenWidth(screenWidth),
      screenHeight(screenHeight)
{
}

void Camera::follow(const Player& player, const Level& level) {
    // Player center
    float playerCenterX = player.position.x + Player::WIDTH / 2;
    float playerCenterY = player.position.y + Player::HEIGHT / 2;
    
    // Screen center
    float screenCenterX = position.x + screenWidth / 2;
    float screenCenterY = position.y + screenHeight / 2;
    
    // Deadzone boundaries
    float deadzoneLeft = screenCenterX - DEADZONE_X / 2;
    float deadzoneRight = screenCenterX + DEADZONE_X / 2;
    float deadzoneTop = screenCenterY - DEADZONE_Y / 2;
    float deadzoneBottom = screenCenterY + DEADZONE_Y / 2;
    
    // Move camera if player exits deadzone
    if (playerCenterX < deadzoneLeft) {
        position.x += (playerCenterX - deadzoneLeft);
    } else if (playerCenterX > deadzoneRight) {
        position.x += (playerCenterX - deadzoneRight);
    }
    
    if (playerCenterY < deadzoneTop) {
        position.y += (playerCenterY - deadzoneTop);
    } else if (playerCenterY > deadzoneBottom) {
        position.y += (playerCenterY - deadzoneBottom);
    }
    
    // Clamp to level boundaries
    int maxX = level.getWidthInPixels() - screenWidth;
    int maxY = level.getHeightInPixels() - screenHeight;
    
    position.x = std::max(0.0f, std::min(position.x, (float)maxX));
    position.y = std::max(0.0f, std::min(position.y, (float)maxY));
}