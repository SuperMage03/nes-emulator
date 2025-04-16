#include "nes-window-sfml.hpp"

NESWindowSFML::NESWindowSFML(const unsigned int& window_width, const unsigned int& window_height, const std::string& window_title): 
    window_(sf::VideoMode({window_width, window_height}), window_title), pixel_({1, 1}) {}

NESWindowSFML::~NESWindowSFML() {
    window_.close();
}

void NESWindowSFML::render() {
    window_.clear(sf::Color::Black);
    for (unsigned int y = 0; y < NES_WINDOW_PIXEL_BUFFER_HEIGHT; y++) {
        for (unsigned int x = 0; x < NES_WINDOW_PIXEL_BUFFER_WIDTH; x++) {
            NESWindow::Colour& pixel_colour = pixel_buffer_[y * NES_WINDOW_PIXEL_BUFFER_WIDTH + x];
            pixel_.setPosition({static_cast<float>(x), static_cast<float>(y)});
            pixel_.setFillColor({pixel_colour.r, pixel_colour.g, pixel_colour.b});
            window_.draw(pixel_);
        }
    }
    window_.display();
}

sf::RenderWindow& NESWindowSFML::getWindow() {
    return window_;
}
