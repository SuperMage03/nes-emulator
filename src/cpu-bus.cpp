#include "cpu-bus.hpp"

CPUBUS::CPUBUS(MOS6502& cpu, MemoryUnit& ram, RP2C02& ppu, const std::unique_ptr<Cartridge>& cartridge): 
    cpu_(cpu), ram_(ram), ppu_(ppu), cartridge_(cartridge), controllers_({nullptr, nullptr}) {
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
        return ppu_.readRegister(address & 0b00000111);
    }

    // Read the controller state
    if ((address == 0x4016) || (address == 0x4017)) {
        uint8_t controller_index = address - 0x4016;
        if (controllers_.at(controller_index)) {
            return controllers_.at(controller_index)->popBitFromShiftRegister();
        }
        return 0;
    }

    // Emulate the mirroring of the APU and I/O registers
    if ((0x4000 <= address) && (address <= 0x401F)) {
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
        return ppu_.writeRegister(address & 0b00000111, data);
    }

    if ((0x4000 <= address) && (address <= 0x401F)) {
        // Emulate the mirroring of the APU and I/O registers
        if (address == 0x4014) {
            // OAM DMA
            uint16_t oam_dma_address = static_cast<uint16_t>(data) << 8;
            for (uint16_t i = 0; i < 0x100; i++) {
                writeBusData(0x2004, readBusData(oam_dma_address + i));
            }
        }
    }

    // Request the controller to load the shift register
    if ((address == 0x4016) || (address == 0x4017)) {
        uint8_t controller_index = address - 0x4016;
        if (controllers_.at(controller_index)) {
            controllers_.at(controller_index)->loadShiftRegister();
            return true;
        }
        return false;
    }

    if ((0x4020 <= address) && (address <= 0x5FFF)) {
        // I don't know what to do here yet
        return false;
    }

    // CPU can't write to the cartridge
    return false;
}

bool CPUBUS::connectController(Controller* controller) {
    if (!controllers_.at(0)) {
        controllers_.at(0) = controller;
        return true;
    }
    if (!controllers_.at(1)) {
        controllers_.at(1) = controller;
        return true;
    }
    return false;
}
