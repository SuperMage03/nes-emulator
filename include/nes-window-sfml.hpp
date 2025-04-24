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
    NESWindowSFML(const uint16_t& frame_rate_limit = NES_WINDOW_FPS, const uint16_t& window_width = NES_WINDOW_SFML_WIDTH, const uint16_t& window_height = NES_WINDOW_SFML_HEIGHT, const std::string& window_title = NES_WINDOW_TITLE);
    ~NESWindowSFML() override;
    void setPixel(const uint16_t& x, const uint16_t& y, const Colour& colour) override;
    void render() override;
    sf::RenderWindow& getWindow();
private:
    sf::RenderWindow window_;
    sf::Texture display_texture_;
    sf::Sprite display_sprite_;
    std::unique_ptr<uint8_t[]> pixel_buffer_;
};

#endif
