#ifndef _CONTROLLER_HPP_
#define _CONTROLLER_HPP_
#include <cstdint>

class Controller {
public:
    enum class Button {
        RIGHT = 0,
        LEFT = 1,
        DOWN = 2,
        UP = 3,
        START = 4,
        SELECT = 5,
        B = 6,
        A = 7,
    };

    /**
     * @brief  Constructor for Controller
     * @param  None
     * @return None
    */
    Controller();

    /**
     * @brief  sets the state of a button
     * @param  button: The button to set
     * @param  isActive: The state of the button
     * @return None
    */
    void setButtonState(const Button& button, bool isActive);

    /**
     * @brief  Presses a button
     * @param  button: The button to press
     * @return None
    */
    void pressButton(const Button& button);

    /**
     * @brief  Releases a button
     * @param  button: The button to release
     * @return None
    */
    void releaseButton(const Button& button);

    /**
     * @brief  Gets the state of the controller buttons
     * @param  None
     * @return The state of the button
    */
    uint8_t getButtonState() const;

    /**
     * @brief  Loads the shift register with the current button state
     * @param  None
     * @return None
    */
    void loadShiftRegister();

    /**
     * @brief  Shifts the shift register to the left and pop out the most significant bit
     * @param  None
     * @return The most significant bit of the shift register
    */
    uint8_t popBitFromShiftRegister();

private:
    uint8_t button_state_;
    uint8_t shift_register_;

};

#endif
