// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes-window-sfml.hpp"
#include "nes.hpp"

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode({256, 224}), "NES Emulator");
    // We will use SFML framerate limit to control the speed of the emulator
    window.setFramerateLimit(60);

    NES nes;
    NESWindowSFML nes_window(window);
    nes.connectDisplayWindow(nes_window);
    nes.loadCartridge("./tests/color_test.nes");

    // run the program as long as the window is open
    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = window.pollEvent()) {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        nes.stepFrame();
        window.display();
    }
    return 0;
}
