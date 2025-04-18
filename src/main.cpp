// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes-window-sfml.hpp"
#include "nes-debug-window.hpp"
#include "nes.hpp"

int main(int argc, char* argv[]) {
    NESWindowSFML nes_window;
    sf::RenderWindow& window = nes_window.getWindow();
    window.setFramerateLimit(60);
    // Create a new NESDebugWindow
    // NESDebugWindow nes_debug_window;

    NES nes;
    nes.connectDisplayWindow(nes_window);
    nes.loadCartridge("./tests/donkey_kong.nes");

    // nes_debug_window.attachNES(&nes);

    // run the program as long as the window is open
    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = window.pollEvent()) {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                switch (key_pressed->scancode) {
                    case sf::Keyboard::Scancode::Escape:
                        window.close();
                        break;
                    case sf::Keyboard::Scancode::Space:
                        nes.stepFrame();
                        break;
                    default:
                        break;
                }
            }
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                const float screenWidth = 256;
                const float screenHeight = 240;
                sf::Vector2u new_size = window.getSize();
                // setup my wanted aspect ratio
                float  heightRatio = screenHeight / screenWidth;
                float  widthRatio = screenWidth / screenHeight;
                if (new_size.y * widthRatio <= new_size.x) {
                    new_size.x = new_size.y * widthRatio;
                }
                else if (new_size.x * heightRatio <= new_size.y) {
                    new_size.y = new_size.x * heightRatio;
                }
                window.setSize(sf::Vector2u(new_size.x, new_size.y));
            }
        }
        nes.stepFrame();
        // nes_debug_window.update();
        nes_window.render();
    }
    return 0;
}
