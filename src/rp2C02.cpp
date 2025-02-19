#include "rp2C02.hpp"

void RP2C02::connectBUS(BUS* target_bus) {
    bus_ = target_bus;
}

uint8_t RP2C02::read(const uint16_t& address) const {
    return bus_->readBusData(address);
}

bool RP2C02::write(const uint16_t& address, const uint8_t& data) {
    return bus_->writeBusData(address, data);
}
