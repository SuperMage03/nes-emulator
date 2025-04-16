#include "nes.hpp"
// Standard Library Headers
#include <fstream>
#include <iostream>
// Project Headers
#include "cartridge.hpp"

NES::NES(): 
    clock_count_(0), cpu_(), ram_(CPU_BUS_RAM_SIZE), 
    ppu_(), vram_(PPU_BUS_NAME_TABLE_SIZE), palette_table_(PPU_BUS_PALETTE_TABLE_SIZE), 
    cartridge_(nullptr), cpu_bus_(cpu_, ram_, ppu_, cartridge_), ppu_bus_(ppu_, vram_, cartridge_) {}

void NES::connectDisplayWindow(NESWindow& window) {
    ppu_.connectDisplayWindow(&window);
}

void NES::loadCartridge(const std::string& path) {
    // Open the file
    std::ifstream nes_rom;
    nes_rom.open(path, std::ios::binary);
    if (!nes_rom.is_open()) {
        std::cerr << "Failed to open the file" << std::endl;
    }

    cartridge_ = Cartridge::makeCartridge(nes_rom);
    cpu_.reset();
}

void NES::releaseCartridge() {
    cartridge_.reset();
}

void NES::clock() {
    ppu_.runCycle();

    // Run CPU cycle every 3 PPU cycles
    if (clock_count_ % 3 == 0) {
        cpu_.runCycle();
    }

    // PPU Request to trigger NMI
	if (ppu_.getNMIFlag()) {
		cpu_.nmi();
        ppu_.setNMIFlag(false);
	}

    // Increase total system clock count
    clock_count_++;
}

void NES::stepFrame() {
    for (uint64_t i = 0; i < 89342; i++) {
        clock();
    }
}
