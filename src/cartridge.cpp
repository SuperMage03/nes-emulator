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
    // ROM file type
    uint8_t file_type = 1;

    // Size of the ROM chunks
    uint16_t prg_rom_chunk_size = 0x0000;
    uint16_t chr_rom_chunk_size = 0x0000;

    switch (file_type) {
        // iNES 1.0
        case 1: {
            prg_rom_chunk_size = 0x4000;
            chr_rom_chunk_size = 0x2000;
        }
        break;
        // iNES 2.0
        case 2: {

        }
        break;
        // Unsupported file type
        default: {
            return nullptr;
        }
    }

    std::unique_ptr<Cartridge> instance = std::unique_ptr<Cartridge>(new Cartridge(mapper_id, header.prg_rom_chunks * prg_rom_chunk_size, header.chr_rom_chunks * chr_rom_chunk_size));
    // Read the program memory and stores it in the instance
    rom_data_stream.read(reinterpret_cast<char*>(instance->prg_rom_memory_.getPointer()), instance->prg_rom_memory_.getSize());
    // Read the pattern memory section and stores it in the instance
    rom_data_stream.read(reinterpret_cast<char*>(instance->chr_memory_.getPointer()), instance->chr_memory_.getSize());
    return std::move(instance);
}

uint8_t Cartridge::readPrgMem(const uint16_t& address) const {
    uint16_t mapped_address = mapper_->mapCPUReadAddress(address);
    if (mapped_address < MAPPER_PRG_RAM_REGION_SIZE) {
        return prg_ram_memory_.read(mapped_address);
    }
    return prg_rom_memory_.read(mapped_address - MAPPER_PRG_RAM_REGION_SIZE);
}

bool Cartridge::writeToPrgMem(const uint16_t& address, const uint8_t& data) {
    uint16_t mapped_address = mapper_->mapCPUWriteAddress(address);
    if (mapped_address < MAPPER_PRG_RAM_REGION_SIZE) {
        prg_ram_memory_.write(mapped_address, data);
        return true;
    }
    // PRG ROM region can't be written
    return false;
}

uint8_t Cartridge::readChrMem(const uint16_t& address) const {
    return chr_memory_.read(mapper_->mapPPUReadAddress(address));
}

bool Cartridge::writeToChrMem(const uint16_t& address, const uint8_t& data) {
    return chr_memory_.write(mapper_->mapPPUWriteAddress(address), data);
}

Cartridge::Cartridge(const uint8_t& mapper_id, const uint32_t& prg_rom_size, const uint32_t& chr_rom_size): 
    prg_ram_memory_(MAPPER_PRG_RAM_REGION_SIZE), prg_rom_memory_(prg_rom_size), chr_memory_(chr_rom_size), 
    mapper_(Mapper::makeMapper(mapper_id, MAPPER_PRG_RAM_REGION_SIZE, prg_rom_size, chr_rom_size)) {}
