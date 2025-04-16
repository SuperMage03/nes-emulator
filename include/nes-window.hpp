#ifndef _NES_WINDOW_HPP_
#define _NES_WINDOW_HPP_
// Standard Library Headers
#include <cstdint>
#include <memory>
// Project Defines
#define NES_WINDOW_PIXEL_BUFFER_WIDTH  256
#define NES_WINDOW_PIXEL_BUFFER_HEIGHT 240

class NESWindow {
public:
    struct Colour {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    // Constructor
    NESWindow();

    // Destructor
    virtual ~NESWindow() = default;
    
    /**
    * @brief  Sets the pixel at the given x and y coordinate
    * @param  x: The x coordinate of the pixel
    * @param  y: The y coordinate of the pixel
    * @param  r: The red value of the pixel
    * @param  g: The green value of the pixel
    * @param  b: The blue value of the pixel
    * @return None
    */
    void setPixel(const uint16_t& x, const uint16_t& y, const Colour& colour);

    /**
    * @brief  Renders the pixel buffer to the display window
    * @param  None
    * @return None
    */
    virtual void render() = 0;

protected:
    std::unique_ptr<Colour[]> pixel_buffer_;
};

#endif
