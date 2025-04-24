#ifndef _NES_DEBUG_WINDOW_HPP_
#define _NES_DEBUG_WINDOW_HPP_
// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes.hpp"
// Project Defines
#define NES_DEBUG_WINDOW_WIDTH 550
#define NES_DEBUG_WINDOW_HEIGHT 800
#define NES_DEBUG_WINDOW_NAME_TABLE_WIDTH 256
#define NES_DEBUG_WINDOW_NAME_TABLE_HEIGHT 240
#define NES_DEBUG_WINDOW_TITLE "NES Debug Window"

class NESDebugWindow {
public:
    /**
     * @brief  Constructor for NESDebugWindow
     * @param  None
     * @return None
    */
    NESDebugWindow();

    /**
     * @brief  Destructor for NESDebugWindow
     * @param  None
     * @return None
    */
    ~NESDebugWindow();

    /**
     * @brief  Connects the NESDebugWindow to the NES system
     * @param  None
     * @return None
    */
    void attachNES(NES* nes);

    /**
     * @brief  Update the NESDebugWindow
     * @param  None
     * @return None
    */
    void update();

private:
    /**
     * @brief  The NES system
    */
    NES* nes_;
    
    /**
     * @brief  The display window for debugging
    */
    sf::RenderWindow window_;

    // Used for displaying the name tables
    sf::Texture name_table_0_texture_;
    sf::Texture name_table_1_texture_;
    sf::Sprite name_table_0_sprite_;
    sf::Sprite name_table_1_sprite_;
    std::unique_ptr<uint8_t[]> name_table_0_pixel_buffer_;
    std::unique_ptr<uint8_t[]> name_table_1_pixel_buffer_;
};

#endif
