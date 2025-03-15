#include "cpu-bus.hpp"

CPUBUS::CPUBUS(MOS6502& cpu, MemoryUnit& ram, RP2C02& ppu, const std::unique_ptr<Cartridge>& cartridge): 
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

    if ((0x4000 <= address) && (address <= 0x401F)) {
        // Emulate the mirroring of the APU and I/O registers
        return 0;
    }

    if ((0x4020 <= address) && (address <= 0x5FFF)) {
        // I don't know what to do here yet
        return 0;
    }

    // Logics for the Cartridge read
    if (!cartridge_) {
        // We can do something here for when the cartridge is not loaded
        return 0;
    }
    return cartridge_->readPrgMem(address - 0x6000);
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

    if ((0x4000 <= address) && (address <= 0x401F)) {
        // Emulate the mirroring of the APU and I/O registers
        return false;
    }

    if ((0x4020 <= address) && (address <= 0x5FFF)) {
        // I don't know what to do here yet
        return false;
    }

    // CPU can't write to the cartridge
    return false;
}
