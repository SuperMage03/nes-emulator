#include "ppu-bus.hpp"

PPUBUS::PPUBUS(RP2C02& ppu, MemoryUnit& name_table, MemoryUnit& palette_table, const std::unique_ptr<Cartridge>& cartridge): 
    ppu_{ppu}, name_table_{name_table}, palette_table_{palette_table}, cartridge_{cartridge} {
    ppu_.connectBUS(this);
}

uint8_t PPUBUS::readBusData(const uint16_t& address) const {
    // Retrieve pattern table data from the Cartridge
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        return cartridge_->readChrMem(address);
    }
    // Retrieve name table data from the Name Table
    if ((0x2000 <= address) && (address <= 0x3EFF)) {
        return name_table_.read((address - 0x2000) % 0x1000);
    }
    // Retrieve palette data from the Palette Table
    if ((0x3F00 <= address) && (address <= 0x3FFF)) {
        return palette_table_.read((address - 0x3F00) % 0x0020);
    }
    // Retrieve mirror of 0x0000 to 0x3FFF
    return readBusData(address - 0x4000);
}

bool PPUBUS::writeBusData(const uint16_t& address, const uint8_t& data) {
    // Write data to the pattern table in the cartridge
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        return cartridge_->writeToChrMem(address, data);
    }
    // Write data to the Name Table
    if ((0x2000 <= address) && (address <= 0x3EFF)) {
        return name_table_.write((address - 0x2000) % 0x1000, data);
    }
    // Write data to the Palette Table
    if ((0x3F00 <= address) && (address <= 0x3FFF)) {
        return palette_table_.write((address - 0x3F00) % 0x0020, data);
    }
    // Write data to the mirror of 0x0000 to 0x3FFF
    return writeBusData(address - 0x4000, data);
}
