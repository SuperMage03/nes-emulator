#include "cpu-bus.hpp"

CPUBUS::CPUBUS(MOS6502& cpu, MemoryUnit& ram, RP2C02& ppu): cpu_(cpu), ram_(ram), ppu_(ppu) {
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
        return ppu_.read(address % CPU_BUS_PPU_SIZE);
    }

    return ram_.read(address);
}

bool CPUBUS::writeBusData(const uint16_t& address, const uint8_t& data) {
    // Check if the address is in the range of the RAM
    if ((0x0000 <= address) && (address <= 0x1FFF)) {
        // Emulate the mirroring of the RAM
        return ram_.write(address % CPU_BUS_RAM_SIZE, data);
    }
    return ram_.write(address, data);
}
