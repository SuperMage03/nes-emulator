#ifndef _NES_WINDOW_HPP_
#define _NES_WINDOW_HPP_
// Standard Library Headers
#include <cstdint>

class NESWindow {
public:
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
    virtual void setPixel(const uint16_t& x, const uint16_t& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) = 0;
};

#endif
