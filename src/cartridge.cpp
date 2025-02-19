#include "cartridge.hpp"

std::unique_ptr<Cartridge> Cartridge::makeCartridge(std::istream& rom_data_stream) {
    Header header;
    // Reads the header
    rom_data_stream.read(reinterpret_cast<char*>(&header), sizeof(Header));
    // Ignore training data
    if (header.mapper1 & 0x04) {
        rom_data_stream.seekg(512, std::ios_base::cur);
    }

    // Determine the mapper ID
    uint8_t mapper_id = (header.mapper2 & 0xF0) | (header.mapper1 >> 4);

    uint8_t file_type = 1;

    switch (file_type) {
        case 1: {
            std::unique_ptr<Cartridge> instance = std::unique_ptr<Cartridge>(new Cartridge(mapper_id, header.prg_rom_chunks * 0x4000, header.chr_rom_chunks * 0x2000));
            // Read the program memory
            rom_data_stream.read(reinterpret_cast<char*>(instance->prg_memory_.getMemoryBlock()), instance->prg_memory_.getSize());
            // Read the pattern memory
            rom_data_stream.read(reinterpret_cast<char*>(instance->chr_memory_.getMemoryBlock()), instance->chr_memory_.getSize());
            return std::move(instance);
        }

        default: {
            // Unsupported file type
            return nullptr;
        }
    }
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

Cartridge::Cartridge(const uint8_t& mapper_id, const uint32_t& program_memory_size, const uint32_t& pattern_memory_size): 
    mapper_(Mapper::makeMapper(mapper_id)), prg_memory_(program_memory_size), chr_memory_(pattern_memory_size) {}
