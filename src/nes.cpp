#include "nes.hpp"
// Standard Library Headers
#include <fstream>
#include <iostream>
// Project Headers
#include "cartridge.hpp"

NES::NES(): 
    cpu_(), ram_(CPU_BUS_RAM_SIZE), 
    ppu_(), name_table_(PPU_BUS_NAME_TABLE_SIZE), palette_table_(PPU_BUS_PALETTE_TABLE_SIZE), 
    cartridge_(nullptr), cpu_bus_(cpu_, ram_, ppu_, cartridge_), ppu_bus_(ppu_, name_table_, palette_table_, cartridge_) {}

void NES::loadCartridge(const std::string& path) {
    // Open the file
    std::ifstream nes_rom;
    nes_rom.open(path, std::ios::binary);
    if (!nes_rom.is_open()) {
        std::cerr << "Failed to open the file" << std::endl;
    }

    cartridge_ = Cartridge::makeCartridge(nes_rom);
}
