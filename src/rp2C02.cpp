#include "rp2C02.hpp"
// REMOVE ME
#include <cstdlib>

void RP2C02::connectDisplayWindow(NESWindow* window) {
    window_ = window;
}

void RP2C02::connectBUS(BUS* target_bus) {
    bus_ = target_bus;
}

void RP2C02::runCycle() {
    cycles_elapsed_++;

    if (window_ != nullptr) {
        window_->setPixel(drawing_col, drawing_row, rand() % 256, rand() % 256, rand() % 256);
    }

    drawing_col++;
    if (drawing_col >= 341) {
        drawing_col = 0;
        drawing_row++;
        if (drawing_row >= 261) {
            drawing_row = -1;
        }
    }
}

uint8_t RP2C02::read(const uint16_t& address) const {
    return bus_->readBusData(address);
}

bool RP2C02::write(const uint16_t& address, const uint8_t& data) {
    return bus_->writeBusData(address, data);
}
