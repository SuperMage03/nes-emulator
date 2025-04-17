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

        // Maps the bus address to the address of the 4 addressing name tables
        uint16_t adressing_name_table_address = (address - 0x2000) % (sizeof(RP2C02::NameTable) * 4);
        
        // Cartridge is not loaded
        if (!cartridge_) {
            return vram_.read(adressing_name_table_address % vram_.getSize());
        }

        // Determine which of the 4 addressing name tables are being accessed
        uint8_t addressing_name_table_index = adressing_name_table_address / sizeof(RP2C02::NameTable);

        switch (cartridge_->getMirrorMode()) {
            case Cartridge::MirrorMode::HORIZONTAL: {
                if ((addressing_name_table_index == 1) || (addressing_name_table_index == 2)) {
                    // Maps the second addressing name table to the first vram name table
                    // Maps the third addressing name table to the second vram name table
                    return vram_.read(adressing_name_table_address - 0x0400);
                }
                if (addressing_name_table_index == 3) {
                    // Mirroring the fourth name table to the second vram name table
                    return vram_.read(adressing_name_table_address - 0x0800);
                }
                // Default case, return the first name table
                return vram_.read(adressing_name_table_address);
            }
            case Cartridge::MirrorMode::VERTICAL: {
                if ((addressing_name_table_index == 2) || (addressing_name_table_index == 3)) {
                    // Maps the third addressing name table to the first vram name table
                    // Maps the fourth addressing name table to the second vram name table
                    return vram_.read(adressing_name_table_address - 0x0800);
                }
                // Default case, return the first name table and second name table
                return vram_.read(adressing_name_table_address);
            }
            default: {
                return vram_.read(adressing_name_table_address % vram_.getSize());
            }
        }
    }

    // Retrieve palette data from the Palette Table
    if ((0x3F00 <= address) && (address <= 0x3FFF)) {
        ppu_.setReadFromDataBuffer(false);
        uint16_t palette_address = (address - 0x3F00) % 0x0020;
        // Transparent Colour Mapping
        if (palette_address % 4 == 0) {
            palette_address &= 0x000F;
        }
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
        // Maps the bus address to the address of the 4 addressing name tables
        uint16_t adressing_name_table_address = (address - 0x2000) % (sizeof(RP2C02::NameTable) * 4);
        
        // Cartridge is not loaded
        if (!cartridge_) {
            return vram_.write(adressing_name_table_address % vram_.getSize(), data);
        }

        // Determine which of the 4 addressing name tables are being accessed
        uint8_t addressing_name_table_index = adressing_name_table_address / sizeof(RP2C02::NameTable);

        switch (cartridge_->getMirrorMode()) {
            case Cartridge::MirrorMode::HORIZONTAL: {
                if ((addressing_name_table_index == 1) || (addressing_name_table_index == 2)) {
                    // Maps the second addressing name table to the first vram name table
                    // Maps the third addressing name table to the second vram name table
                    return vram_.write(adressing_name_table_address - 0x0400, data);
                }
                if (addressing_name_table_index == 3) {
                    // Mirroring the fourth name table to the second vram name table
                    return vram_.write(adressing_name_table_address - 0x0800, data);
                }
                // Default case, return the first name table
                return vram_.write(adressing_name_table_address, data);
            }
            case Cartridge::MirrorMode::VERTICAL: {
                if ((addressing_name_table_index == 2) || (addressing_name_table_index == 3)) {
                    // Maps the third addressing name table to the first vram name table
                    // Maps the fourth addressing name table to the second vram name table
                    return vram_.write(adressing_name_table_address - 0x0800, data);
                }
                // Default case, return the first name table and second name table
                return vram_.write(adressing_name_table_address, data);
            }
            default: {
                return vram_.write(adressing_name_table_address % vram_.getSize(), data);
            }
        }
    }

    // Write data to the Palette Table
    if ((0x3F00 <= address) && (address <= 0x3FFF)) {
        uint16_t palette_address = (address - 0x3F00) % 0x0020;
        // Transparent Colour Mapping
        if (palette_address % 4 == 0) {
            palette_address &= 0x000F;
        }
        return ppu_.writePaletteTable(palette_address, data);
    }
    
    // Write data to the mirror of 0x0000 to 0x3FFF
    return writeBusData((address - 0x4000) % 0x4000, data);
}
