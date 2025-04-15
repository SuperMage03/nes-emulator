// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes-window-sfml.hpp"
#include "nes.hpp"

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode({600, 600}), "NES Emulator");
    // We will use SFML framerate limit to control the speed of the emulator
    window.setFramerateLimit(60);

    NES nes;
    NESWindowSFML nes_window(window);
    nes.connectDisplayWindow(nes_window);
    nes.loadCartridge("./tests/pac_man.nes");

    const auto onClose = [&window](const sf::Event::Closed&) {
        window.close();
    };
    
    const auto onKeyPressed = [&window](const sf::Event::KeyPressed& keyPressed) {
        if (keyPressed.scancode == sf::Keyboard::Scancode::Escape)
            window.close();
    };

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
                    default:
                        break;
                
                }
            }
        }
        nes.stepFrame();
        window.display();
    }
    return 0;
}
