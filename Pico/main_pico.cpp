#include "pico/stdlib.h"
#include "../Shared/game/Game.h"
#include "../Shared/game/level/Level.h"
#include "PicoRenderer.h"
#include "PicoInput.h"
#include "PicoHaptics.h"
#include "PicoTimer.h"
#include "assets/level1_data.h"
#include "assets/level2_data.h"

int main() {
    stdio_init_all();
    
    PicoRenderer renderer;
    PicoInput input;
    PicoHaptics haptics;
    PicoTimer timer;
    
    Game game(&renderer, &input, &haptics, &timer);
    game.init();

    game.getLevel().loadFromBinaryData(level1_tiles, level1_width, level1_height, &level1_metadata);
    game.syncEntitiesFromCurrentLevel("../levels/Level1.csv");

    while (true) {
        game.update();
        game.render();
    }
    
    return 0;
}
