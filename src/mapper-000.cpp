#include "mapper-000.hpp"

Mapper000::Mapper000(const uint32_t& prg_ram_size, const uint32_t& prg_rom_size, const uint32_t& chr_rom_size): 
    Mapper(prg_ram_size, prg_rom_size, chr_rom_size) {}

uint16_t Mapper000::mapCPUReadAddress(const uint16_t& address) const {
    if (address < MAPPER_PRG_RAM_REGION_SIZE) {
        return address;
    }
    return MAPPER_PRG_RAM_REGION_SIZE + ((address - MAPPER_PRG_RAM_REGION_SIZE) % prg_rom_size_);
}

uint16_t Mapper000::mapCPUWriteAddress(const uint16_t& address) const {
    return mapCPUReadAddress(address);
}

uint16_t Mapper000::mapPPUReadAddress(const uint16_t& address) const {
    return address;
}

uint16_t Mapper000::mapPPUWriteAddress(const uint16_t& address) const {
    return address;
}
