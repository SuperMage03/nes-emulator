#include "cpu-bus.hpp"

CPUBUS::CPUBUS(MOS6502& cpu, MemoryUnit& ram, RP2C02& ppu, std::unique_ptr<Cartridge>& cartridge): 
    cpu_(cpu), ram_(ram), ppu_(ppu), cartridge_(cartridge) {
    cpu_.connectBUS(this);
}

uint8_t CPUBUS::readBusData(const uint16_t& address) const {
    // Check if the address is in the range of the RAM
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        // Emulate the mirroring of the RAM
        return ram_.read(address % CPU_BUS_RAM_SIZE);
    }
    
    // Check if the address is in the range of the PPU registers
    if ((0x2000 <= address) && (address <= 0x3FFF)) {
        // Emulate the mirroring of the PPU registers
        return ppu_.read((address - 0x2000) % CPU_BUS_PPU_SIZE);
    }

    // Check if the address is in the range of the Cartridge
    if ((0x4020 <= address) && (address <= 0xFFFF)) {
        return cartridge_->readPrgMem(address - 0x4020);
    }

    return 0;
}

bool CPUBUS::writeBusData(const uint16_t& address, const uint8_t& data) {
    // Check if the address is in the range of the RAM
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        // Emulate the mirroring of the RAM
        return ram_.write(address % CPU_BUS_RAM_SIZE, data);
    }

    // Check if the address is in the range of the PPU registers
    if ((0x2000 <= address) && (address <= 0x3FFF)) {
        // Emulate the mirroring of the PPU registers
        return ppu_.write((address - 0x2000) % CPU_BUS_PPU_SIZE, data);
    }

    // Check if the address is in the range of the Cartridge
    if ((0x4020 <= address) && (address <= 0xFFFF)) {
        return cartridge_->writeToMapper(address - 0x4020, data);
    }

    return false;
}
