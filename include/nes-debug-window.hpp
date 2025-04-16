#ifndef _NES_DEBUG_WINDOW_HPP_
#define _NES_DEBUG_WINDOW_HPP_
// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// Project Headers
#include "nes.hpp"
// Project Defines
#define NES_DEBUG_WINDOW_WIDTH 700
#define NES_DEBUG_WINDOW_HEIGHT 800
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
};

#endif
