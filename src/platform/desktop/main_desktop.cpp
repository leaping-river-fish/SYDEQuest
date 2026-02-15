#include <SDL2/SDL.h>
#include "game/Game.h"
#include "DesktopRenderer.h"
#include "DesktopInput.h"
#include "DesktopHaptics.h"
#include "DesktopTimer.h"

int main(int argc, char* argv[]) {
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }
    
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 240;
    const int SCALE = 3;
    
    SDL_Window* window = SDL_CreateWindow(
        "Platformer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    // Set logical size for pixel-perfect scaling
    SDL_RenderSetLogicalSize(sdlRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Create platform implementations
    DesktopRenderer renderer(sdlRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    DesktopInput input;
    DesktopHaptics haptics;
    DesktopTimer timer;
    
    // Create game
    Game game(&renderer, &input, &haptics, &timer);
    game.init();
    
    // Game loop
    while (!input.shouldQuit()) {
        game.update();
        game.render();
    }
    
    // Cleanup
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}