#include "nes-window-sfml.hpp"

NESWindowSFML::NESWindowSFML(const unsigned int& window_width, const unsigned int& window_height, const std::string& window_title): 
    window_(sf::VideoMode({window_width, window_height}), window_title), pixel_({1, 1}) {}

NESWindowSFML::~NESWindowSFML() {
    window_.close();
}

void NESWindowSFML::setPixel(const uint16_t& x, const uint16_t& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) {
    pixel_.setPosition({static_cast<float>(x), static_cast<float>(y)});
    pixel_.setFillColor({r, g, b});
    window_.draw(pixel_);
}

sf::RenderWindow& NESWindowSFML::getWindow() {
    return window_;
}
