#ifndef _NES_WINDOW_SFML_HPP_
#define _NES_WINDOW_SFML_HPP_
// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes-window.hpp"
// Project Defines
#define NES_WINDOW_SFML_WIDTH 256
#define NES_WINDOW_SFML_HEIGHT 240
#define NES_WINDOW_TITLE "NES Emulator Window"

class NESWindowSFML : public NESWindow {
public:
    NESWindowSFML(const unsigned int& window_width = NES_WINDOW_SFML_WIDTH, const unsigned int& window_height = NES_WINDOW_SFML_HEIGHT, const std::string& window_title = NES_WINDOW_TITLE);
    ~NESWindowSFML() override;
    void render() override;
    sf::RenderWindow& getWindow();
private:
    sf::RenderWindow window_;
    sf::RectangleShape pixel_;
};

#endif
