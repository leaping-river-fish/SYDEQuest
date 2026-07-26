# SYDEQUEST



## Project Description



This project is the capstone work for SYDE 263, combining **digital fabrication**, **embedded systems**, and **original game software** into one integrated system: a custom handheld console that runs an original platformer, **SYDEQuest**. The physical side of the project follows a full design cycle—from early cardboard fit checks and layout planning through laser-cut acrylic, 3D-printed enclosure parts, wiring, and a finished assembly that houses a Raspberry Pi Pico 2, display, joystick and buttons, charging and battery hardware, and haptic feedback. The goal is not only a playable device but a demonstration of how CAD, sensors, actuators, and firmware come together in a single product.



![Early prototype enclosure planning](Prototype.jpeg)

[Early prototype enclosure planning]


**SYDEQuest** is a 2D platformer written in **C++** with a **desktop-first** architecture (SDL2) so gameplay, physics, and content can be developed and debugged on PC before the same core game logic is targeted at the Pico 2. The game loads **tile-based levels** from CSV files, uses a scrolling camera, and includes platformer physics, collision, projectiles, melee and ranged enemies, health pickups, and animated portals. Collectible **objectives** in the world are themed to the course build: items such as the charger, enclosure, haptic module, parts, screen, and Pico—linking the virtual quest to the real hardware stack.



![Level 1 layout (tile map)](Level1.png)

[Level 1 layout (tile map)]


![In-game SYDEQuest screenshot](sydequest7.png)

[In-game SYDEQuest screenshot]


On the hardware side, input comes from an **analog joystick** and **buttons**; output includes a small **SPI display** for the game view and a **vibration motor** for haptics on events such as damage or collisions. An enclosure designed for hardware visibility, access to ports, and secure mounting of the Pico, screen, and controls. The photos below show the assembled console from the front and back, including the user-facing controls and display and the rear with enclosure details.



![Finished handheld console (front)](FinalF.jpeg)

[Finished handheld console (front)]


![Finished handheld console (back)](FinalB.jpeg)

[Finished handheld console (back)]


Taken together, SYDEQuest and the handheld console satisfy the course emphasis on **parametric CAD**, **sensor and actuator integration**, **intermediate fabrication** (print, laser cut, assembly), and **embedded programming**—delivering both a playable game and a tangible device that runs that game on real hardware.

## Gameplay

![Level1](Level1.gif)

![Level2](Level2.gif)

![Boss](Boss.gif)

![Level3](Level3.gif)


## Bill of Materials

**Table 1: Bill of Materials**

| Quantity | Description | Source | Cost (CAD $) |
|----------|---------------|--------|----------------|
| 1 | Raspberry Pi Pico 2 (RP2350) | Amazon | 13.96 |
| 1 | Pico-Restouch-LCD-2.8 | Amazon | 28.90 |
| 3 | Momentary tactile push buttons | Amazon | 1.68 |
| 1 | Dual Axis Potentiometer Joystick | Amazon | 6.00 |
| 1 | Coin vibration motor | Amazon | 1.49 |
| 1 | DRV2605 Haptic Driver | Amazon | 14.85 |
| 1 | PLA filament (enclosure walls) | Local | 1.10 |
| 1 | Acrylic sheet / laser-cut parts (as designed) | Workshop | 5.00 |
| 30 | Wires | Local | 0.43 |
| 2 | 5x7 Perfboard | Local | 3.20 |
| 4 | M2.5 x 10mm | Local | 1.80 |
| 8 | M2.5 x 6mm | Local | 2.00 |
| 8 | M2 x 10mm | Local | 1.60 | 
||| **Total** | **82.01** |

*Costs are approximate retail prices in Canadian dollars; costs may vary with exact parts chosen.*

## Reflection

### What
I built a handheld game console consisting of a the typical screen, joystick, four buttons (only 3 now). I also designed and programmed the game that the handheld console will run, SYDEQuest. A few things that went well: the process of programming the game was not too bad, of course debugging was tedious but I enjoyed play testing the game. Designing the sprites for the game was also pretty fun, it was my first time actually trying pixel art. Planning and assembling the actual enclosure of the handheld was very satisfying as I could see everything come together. The hardware and embedded systems were at first fun to put together and solder, but then it got frustrating. 

Moving on the bad experiences: I didn't realize that the Pico did not have enough RAM to be able to run a game with a framebuffer, which caused me to purchase the Pico 2. This lead to my second issue with soldering since I soldered the Pico 2 at home with my own solder which differed from the one in lab, it definitely caused the solder to lose some of its properties when I mixed my solder with the one in the lab. As a result, wires just kept falling out or not sticking enough. Another thing was that I definitely developed my game too far at first before trying to test it on the hardware, this was due to me placing the orders on my parts really late. Consequently, when I tried debugging it was a nightmare figuring out what was actually going wrong since there were so many variables.  

The result: After many hours of debugging (20+) and testing, I finally got the game to run smoothly on my handheld console. I finally had a proper development pipeline: develop a feature, test on pc, migrate to pico, compare and debug. Which made making the pico game actually doable. I now have a proper handheld console that fits together, with a game that can be updated and uploaded to the pico easily.

### So What
The significance of this project for me is that my dream would be to work as a game developer. This project was a good introduction into game design, simple enough to complete while working on hardware, and allowed me to design assets. This project was also significant as it was my first time actually building something physical of my own design. I wanted to see how far I could take it so I chose something I was passionate about. I wanted to prove that I could also do hardware and not just software. Moreover, I wanted to make something that I could let others enjoy, not just something that would just sit in my room.

### Now What
Moving forward, I want to add a charging system so that the game console can actually be used outside of the lab. I've already purchased the 3.7V rechargeable Li-Po battery and charging module, but I did not do enough research to realize that I needed some capacitors to make it run smoothly so did not incorporate it in the final design. I also just did not have enough time. If I had started earlier I could've definitely ordered the capacitors in time. I would also like to incorporate a power switch so that I could turn the console off and save energy. I would need to reprint the walls to add a charging port and to add a slot for the stylus that came with my display screen. Adding a capacitor between the pico and haptic driver would also smooth the experience as well as prevent damage to the pico from any voltage spikes. The nature of microcontrollers allows me to develop more games that could be uploaded and ran on this console, which is something that I plan on doing. 

# Image Appendix

## Debugging

**Table 2: Debugging**

| Photo | Preview |
|-------|---------|
| Debugging1 | ![Debugging1](Debugging1.jpeg) |
| Debugging2 | ![Debugging2](Debugging2.jpeg) |
| Debugging3 | ![Debugging3](Debugging3.jpeg) |
| Debugging4 | ![Debugging4](Debugging4.jpeg) |
| Debugging5 | ![Debugging5](Debugging5.jpeg) |
| Debugging6 | ![Debugging6](Debugging6.jpeg) |

## Sprites

**Table 3: SYDEQuest sprite assets**

| Sprite | Preview |
|--------|---------|
| AlternatingWalk | ![AlternatingWalk](SydeQuest/assets/AlternatingWalk.png) |
| Circle | ![Circle](SydeQuest/assets/Circle.png) |
| CalcQuiz | ![CalcQuiz](SydeQuest/assets/CalcQuiz.png) |
| FullTerrainSpriteSheet | ![FullTerrain](SydeQuest/assets/FullTerrainSpriteSheet.png) |
| Energy | ![Energy](SydeQuest/assets/Energy.png) |
| EnergyDrink | ![EnergyDrink](SydeQuest/assets/EnergyDrink.png) |
| PortalSpriteSheet | ![Portal](SydeQuest/assets/PortalSpriteSheet.png) |
| ScreenSprite | ![Screen](SydeQuest/assets/ScreenSprite.png) |
| EnclosureSprite | ![Enclosure](SydeQuest/assets/EnclosureSprite.png) |
| ChargerSprite | ![Charger](SydeQuest/assets/ChargerSprite.png) |
| PartsSprite | ![Parts](SydeQuest/assets/PartsSprite.png) |
| HapticSprite | ![Haptic](SydeQuest/assets/HapticSprite.png) |
| PicoSprite | ![Pico](SydeQuest/assets/PicoSprite.png) |
| Boss | ![Boss](SydeQuest/assets/Boss.png) |
| Calvin Young | ![Calvin Young](SydeQuest/assets/Calvin%20Young.png) |
| GameOver | ![GameOver](SydeQuest/assets/GameOver.png) |
| IntegralSpinLeft | ![IntegralSpinLeft](SydeQuest/assets/IntegralSpinLeft.png) |
| IntegralSpinRight | ![IntegralSpinRight](SydeQuest/assets/IntegralSpinRight.png) |
| JumpSprite | ![JumpSprite](SydeQuest/assets/JumpSprite.png) |
| PencilSpinLeft | ![PencilSpinLeft](SydeQuest/assets/PencilSpinLeft.png) |
| PencilSpinRight | ![PencilSpinRight](SydeQuest/assets/PencilSpinRight.png) |
| Robert Hunter | ![Robert Hunter](SydeQuest/assets/Robert%20Hunter.png) |
| Rustball | ![Rustball](SydeQuest/assets/Rustball.png) |
| RustShrapnel | ![RustShrapnel](SydeQuest/assets/RustShrapnel.png) |
| SeanSpeziale | ![SeanSpeziale](SydeQuest/assets/SeanSpeziale.png) |
| SYDEQuest | ![SYDEQuest](SydeQuest/assets/SYDEQuest.png) |
| Trophy | ![Trophy](SydeQuest/assets/Trophy.png) |
