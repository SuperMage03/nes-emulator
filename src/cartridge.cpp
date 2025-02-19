#include "cartridge.hpp"

std::unique_ptr<Cartridge> Cartridge::makeCartridge() {
    return nullptr;
}

uint8_t Cartridge::readPrgMem(const uint16_t& address) const {
    return prg_memory_.read(mapper_->mapCPUReadAddress(address));
}

bool Cartridge::writeToMapper(const uint16_t& address, const uint8_t& data) {
    return false;
}

uint8_t Cartridge::readChrMem(const uint16_t& address) const {
    return chr_memory_.read(mapper_->mapPPUReadAddress(address));
}

bool Cartridge::writeToChrMem(const uint16_t& address, const uint8_t& data) {
    return chr_memory_.write(mapper_->mapPPUWriteAddress(address), data);
}
