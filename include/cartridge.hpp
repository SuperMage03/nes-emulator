#ifndef _CARTRIDGE_HPP_
#define _CARTRIDGE_HPP_
// Standard Library Headers
#include <istream>
#include <memory>
#include <cstdint>
// Project Headers
#include "mapper.hpp"
#include "memory-unit.hpp"

class Cartridge {
public:
    enum class MirrorMode {
        HORIZONTAL       = 0,
        VERTICAL         = 1,
        SINGLE_SCREEN_LO = 2,
        SINGLE_SCREEN_HI = 3,
    };

    // iNES Header
    struct Header {
        char name[4];
        uint8_t prg_rom_chunks;
        uint8_t chr_rom_chunks;
        uint8_t mapper1;
        uint8_t mapper2;
        uint8_t prg_ram_size;
        uint8_t tv_system1;
        uint8_t tv_system2;
        char unused[5];
    };

    /**
    * @brief  Factory Method for creating an instance of a cartridge
    * @param  rom_data_stream: The data stream of the ROM
    * @return A unique pointer to the created Cartridge instance
    */
    static std::unique_ptr<Cartridge> makeCartridge(std::istream& rom_data_stream);

    /**
    * @brief  Reads the program rom and ram data from the Cartridge at the address
    * @param  address: The address to read from
    * @return Data read from the Cartridge
    */
    uint8_t readPrgMem(const uint16_t& address) const;

    /**
    * @brief  Writes data to the Cartridge WRAM at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool writeToPrgMem(const uint16_t& address, const uint8_t& data);

    /**
    * @brief  Reads the pattern table data from the Cartridge at the address
    * @param  address: The address to read from
    * @return Data read from the Cartridge
    */
    uint8_t readChrMem(const uint16_t& address) const;

    /**
    * @brief  Writes data to the Cartridge Pattern Table at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool writeToChrMem(const uint16_t& address, const uint8_t& data);

    /**
    * @brief  Gets the mirror mode of the cartridge
    * @param  None
    * @return The mirror mode of the cartridge
    */
    MirrorMode getMirrorMode() const;

protected:
    // Constructor
    explicit Cartridge(const uint8_t& mapper_id, const uint32_t& prg_rom_size, const uint32_t& chr_rom_size, const MirrorMode& mirror_mode);

private:
    MemoryUnit prg_ram_memory_;
    MemoryUnit prg_rom_memory_;
    MemoryUnit chr_memory_;
    std::unique_ptr<Mapper> mapper_;
    MirrorMode mirror_mode_;
};

#endif
