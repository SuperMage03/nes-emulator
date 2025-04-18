#include "controller.hpp"

Controller::Controller(): button_state_(0), shift_register_(0) {}

void Controller::setButtonState(const Button& button, bool isActive) {
    button_state_ &= ~(1 << static_cast<uint8_t>(button));
    button_state_ |= (isActive << static_cast<uint8_t>(button));
}

void Controller::pressButton(const Button& button) {
    button_state_ |= (1 << static_cast<uint8_t>(button));
}

void Controller::releaseButton(const Button& button) {
    button_state_ &= ~(1 << static_cast<uint8_t>(button));
}

uint8_t Controller::getButtonState() const {
    return button_state_;
}

void Controller::loadShiftRegister() {
    shift_register_ = button_state_;
}

uint8_t Controller::popBitFromShiftRegister() {
    uint8_t bit = (shift_register_ & 0x80) > 0;
    shift_register_ <<= 1;
    return bit;
}
