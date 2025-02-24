#include "cartridge.hpp"
#include <cstdint>

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
    // ROM file type
    uint8_t file_type = 1;

    // Size of the ROM chunks
    uint16_t prg_rom_chunk_size = 0x0000;
    uint16_t chr_rom_chunk_size = 0x0000;

    switch (file_type) {
        case 1: {
            prg_rom_chunk_size = 0x4000;
            chr_rom_chunk_size = 0x2000;
        }
        break;

        default: {
            // Unsupported file type
            return nullptr;
        }
    }

    std::unique_ptr<Cartridge> instance = std::unique_ptr<Cartridge>(new Cartridge(mapper_id, header.prg_rom_chunks * prg_rom_chunk_size, header.chr_rom_chunks * chr_rom_chunk_size));
    // Read the program memory
    rom_data_stream.read(reinterpret_cast<char*>(instance->prg_memory_.getPointer()), instance->prg_memory_.getSize());
    // Read the pattern memory
    rom_data_stream.read(reinterpret_cast<char*>(instance->chr_memory_.getPointer()), instance->chr_memory_.getSize());
    return std::move(instance);
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
