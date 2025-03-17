#include "nes-window-sfml.hpp"

NESWindowSFML::NESWindowSFML(sf::RenderWindow& window): 
    window_(window), pixel_({1, 1}) {}

void NESWindowSFML::setPixel(const uint16_t& x, const uint16_t& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) {
    pixel_.setPosition({static_cast<float>(x), static_cast<float>(y)});
    pixel_.setFillColor({r, g, b});
    window_.draw(pixel_);
}
