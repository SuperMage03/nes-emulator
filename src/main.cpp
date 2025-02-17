#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode({256, 224}), "NES Emulator");

    sf::RectangleShape pixel({1, 1});

    // run the program as long as the window is open
    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = window.pollEvent()) {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        window.clear();
        unsigned int counter = 0;
        for (float x = 0.0f; x < 256.0f; x += 1.0f) {
            for (float y = 0.0f; y < 224.0f; y += 1.0f) {
                pixel.setPosition({x, y});
                if (counter % 2 == 0) {
                    pixel.setFillColor({0, 0, 0});
                }
                else {
                    pixel.setFillColor({255, 255, 255});
                }
                window.draw(pixel);
                counter++;
            }
            counter++;
        }
        window.display();
    }
    return 0;
}
