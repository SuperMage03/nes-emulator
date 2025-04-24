#include "mapper.hpp"
// Mapper Child Classes
#include "mapper-000.hpp"

std::unique_ptr<Mapper> Mapper::makeMapper(const uint8_t& mapper_id, const uint32_t& prg_ram_size, const uint32_t& prg_rom_size, const uint32_t& chr_rom_size) {
    switch (mapper_id) {
        case 0x00: {
            return std::make_unique<Mapper000>(prg_ram_size, prg_rom_size, chr_rom_size);
        }
        default: {
            return nullptr;
        }
    }
}

Mapper::Mapper(const uint32_t& prg_ram_size, const uint32_t& prg_rom_size, const uint32_t& chr_rom_size): 
    prg_ram_size_(prg_rom_size), prg_rom_size_(prg_rom_size), chr_rom_size_(chr_rom_size) {}
