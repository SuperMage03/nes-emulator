#include "nes-window-sfml.hpp"
#include "nes-window.hpp"

NESWindowSFML::NESWindowSFML(const uint16_t& frame_rate_limit, const uint16_t& window_width, const uint16_t& window_height, const std::string& window_title): 
    window_(sf::VideoMode(sf::Vector2u{window_width, window_height}), window_title),
    display_texture_(sf::Vector2u{NES_WINDOW_WIDTH, NES_WINDOW_HEIGHT}),
    display_sprite_(display_texture_),
    pixel_buffer_(std::make_unique<uint8_t[]>(NES_WINDOW_WIDTH * NES_WINDOW_HEIGHT * 4)) {
    // Set window frame rate limit
    window_.setFramerateLimit(frame_rate_limit);
}

NESWindowSFML::~NESWindowSFML() {
    window_.close();
}

void NESWindowSFML::setPixel(const uint16_t& x, const uint16_t& y, const Colour& colour) {
    if (x < NES_WINDOW_WIDTH && y < NES_WINDOW_HEIGHT) {
        pixel_buffer_[(y * NES_WINDOW_WIDTH + x) * 4 + 0] = colour.r;
        pixel_buffer_[(y * NES_WINDOW_WIDTH + x) * 4 + 1] = colour.g;
        pixel_buffer_[(y * NES_WINDOW_WIDTH + x) * 4 + 2] = colour.b;
        pixel_buffer_[(y * NES_WINDOW_WIDTH + x) * 4 + 3] = 255; // Solid Alpha
    }
}

void NESWindowSFML::render() {
    window_.clear(sf::Color::Black);
    display_texture_.update(pixel_buffer_.get());
    window_.draw(display_sprite_);
    window_.display();
}

sf::RenderWindow& NESWindowSFML::getWindow() {
    return window_;
}
