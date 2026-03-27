#include "Camera.h"
#include "../player/Player.h"
#include "Level.h"
#include <algorithm>

Camera::Camera(int screenWidth, int screenHeight)
    : position(0.0f, 0.0f),
      screenWidth(screenWidth),
      screenHeight(screenHeight)
{
}

void Camera::follow(const Player& player, const Level& level) {
#ifdef PLATFORM_PICO
    fixed_t halfPlayerWidth = Player::WIDTH >> 1;
    fixed_t halfPlayerHeight = Player::HEIGHT >> 1;
    fixed_t halfScreenWidth = TO_FIXED(screenWidth / 2.0f);
    fixed_t halfScreenHeight = TO_FIXED(screenHeight / 2.0f);
    fixed_t halfDeadzoneX = TO_FIXED(DEADZONE_X / 2.0f);
    fixed_t halfDeadzoneY = TO_FIXED(DEADZONE_Y / 2.0f);
    
    // Player center
    fixed_t playerCenterX = player.position.x + halfPlayerWidth;
    fixed_t playerCenterY = player.position.y + halfPlayerHeight;
#else
    // Player center
    float playerCenterX = player.position.x + Player::WIDTH / 2;
    float playerCenterY = player.position.y + Player::HEIGHT / 2;
#endif
    
#ifdef PLATFORM_PICO
    // Screen center
    fixed_t screenCenterX = position.x + halfScreenWidth;
    fixed_t screenCenterY = position.y + halfScreenHeight;
    
    // Deadzone boundaries
    fixed_t deadzoneLeft = screenCenterX - halfDeadzoneX;
    fixed_t deadzoneRight = screenCenterX + halfDeadzoneX;
    fixed_t deadzoneTop = screenCenterY - halfDeadzoneY;
    fixed_t deadzoneBottom = screenCenterY + halfDeadzoneY;
    
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
    
    position.x = std::max(TO_FIXED(0.0f), std::min(position.x, TO_FIXED(static_cast<float>(maxX))));
    position.y = std::max(TO_FIXED(0.0f), std::min(position.y, TO_FIXED(static_cast<float>(maxY))));
#else
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
    
    position.x = std::max(0.0f, std::min(position.x, static_cast<float>(maxX)));
    position.y = std::max(0.0f, std::min(position.y, static_cast<float>(maxY)));
#endif
}