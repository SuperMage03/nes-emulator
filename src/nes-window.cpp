#include "nes-window.hpp"

NESWindow::NESWindow(): pixel_buffer_{std::make_unique<Colour[]>(NES_WINDOW_PIXEL_BUFFER_WIDTH * NES_WINDOW_PIXEL_BUFFER_HEIGHT)} {
    for (uint16_t i = 0; i < NES_WINDOW_PIXEL_BUFFER_WIDTH * NES_WINDOW_PIXEL_BUFFER_HEIGHT; i++) {
        pixel_buffer_[i] = {0, 0, 0};
    }
}

void NESWindow::setPixel(const uint16_t& x, const uint16_t& y, const Colour& colour) {
    if (x < NES_WINDOW_PIXEL_BUFFER_WIDTH && y < NES_WINDOW_PIXEL_BUFFER_HEIGHT) {
        pixel_buffer_[y * NES_WINDOW_PIXEL_BUFFER_WIDTH + x] = colour;
    }
}
