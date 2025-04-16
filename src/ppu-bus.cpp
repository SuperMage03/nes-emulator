#include "ppu-bus.hpp"

PPUBUS::PPUBUS(RP2C02& ppu, MemoryUnit& vram, const std::unique_ptr<Cartridge>& cartridge): 
    ppu_{ppu}, vram_{vram}, cartridge_{cartridge} {
    ppu_.connectBUS(this);
}

uint8_t PPUBUS::readBusData(const uint16_t& address) const {
    // Retrieve pattern table data from the Cartridge
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        ppu_.setReadFromDataBuffer(true);
        if (!cartridge_) {
            // We can do something here for when the cartridge is not loaded
            return 0;
        }
        return cartridge_->readChrMem(address);
    }

    // Retrieve name table data from the Name Table
    if ((0x2000 <= address) && (address <= 0x3EFF)) {
        ppu_.setReadFromDataBuffer(true);
        return vram_.read((address - 0x2000) % 0x1000);
    }

    // Retrieve palette data from the Palette Table
    if ((0x3F00 <= address) && (address <= 0x3FFF)) {
        ppu_.setReadFromDataBuffer(false);
        uint16_t palette_address = (address - 0x3F00) % 0x0020;
        if (palette_address == 0x0010) palette_address = 0x0000;
		if (palette_address == 0x0014) palette_address = 0x0004;
		if (palette_address == 0x0018) palette_address = 0x0008;
		if (palette_address == 0x001C) palette_address = 0x000C;
        return ppu_.readPaletteTable(palette_address);
    }

    // Retrieve mirror of 0x0000 to 0x3FFF
    return readBusData((address - 0x4000) % 0x4000);
}

bool PPUBUS::writeBusData(const uint16_t& address, const uint8_t& data) {
    // Write data to the pattern table in the cartridge
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        if (!cartridge_) {
            // We can do something here for when the cartridge is not loaded
            return false;
        }
        return cartridge_->writeToChrMem(address, data);
    }

    // Write data to the Name Table
    if ((0x2000 <= address) && (address <= 0x3EFF)) {
        return vram_.write((address - 0x2000) % 0x1000, data);
    }

    // Write data to the Palette Table
    if ((0x3F00 <= address) && (address <= 0x3FFF)) {
        uint16_t palette_address = (address - 0x3F00) % 0x0020;
        if (palette_address == 0x0010) palette_address = 0x0000;
		if (palette_address == 0x0014) palette_address = 0x0004;
		if (palette_address == 0x0018) palette_address = 0x0008;
		if (palette_address == 0x001C) palette_address = 0x000C;
        return ppu_.writePaletteTable(palette_address, data);
    }
    
    // Write data to the mirror of 0x0000 to 0x3FFF
    return writeBusData((address - 0x4000) % 0x4000, data);
}
