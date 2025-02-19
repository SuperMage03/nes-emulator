#include "nes.hpp"
#include "cartridge.hpp"

NES::NES(): 
    cpu_(), ram_(CPU_BUS_RAM_SIZE), 
    ppu_(), name_table_(PPU_BUS_NAME_TABLE_SIZE), palette_table_(PPU_BUS_PALETTE_TABLE_SIZE), 
    cartridge_(Cartridge::makeCartridge()), cpu_bus_(cpu_, ram_, ppu_, cartridge_), ppu_bus_(ppu_, name_table_, palette_table_, cartridge_) {}

void NES::loadCartridge(const std::string& path) {
    uint8_t mapper_id = 0;
    uint8_t program_memory_bank_count = 0;
    uint8_t pattern_memory_bank_count = 0;
    

}
