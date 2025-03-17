#ifndef _NES_WINDOW_SFML_HPP_
#define _NES_WINDOW_SFML_HPP_
// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes-window.hpp"

class NESWindowSFML : public NESWindow {
public:
    NESWindowSFML(sf::RenderWindow& window_);
    void setPixel(const uint16_t& x, const uint16_t& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) override;
private:
    sf::RenderWindow& window_;
    sf::RectangleShape pixel_;
};

#endif
