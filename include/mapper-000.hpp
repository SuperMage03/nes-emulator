#ifndef _MAPPER_000_HPP_
#define _MAPPER_000_HPP_
#include "mapper.hpp"

class Mapper000: public Mapper {
public:
    // Constructor
    Mapper000(const uint32_t& prg_ram_size, const uint32_t& prg_rom_size, const uint32_t& chr_rom_size);

    /*
    * @brief  Returns the mapped address for the CPU to read from the Cartridge
    * @param  address: The address to read from
    * @return Mapped address
    */
    uint16_t mapCPUReadAddress(const uint16_t& address) const override;

    /*
    * @brief  Returns the mapped address for the CPU to write from the Cartridge
    * @param  address: The address to write to
    * @return Mapped Address
    */
    uint16_t mapCPUWriteAddress(const uint16_t& address) const override;

    /*
    * @brief  Returns the mapped address for the PPU to read from the Cartridge
    * @param  address: The address to read from
    * @return Mapped Address
    */
    uint16_t mapPPUReadAddress(const uint16_t& address) const override;

    /*
    * @brief  Returns the mapped address for the PPU to write from the Cartridge
    * @param  address: The address to write to
    * @return Mapped Address
    */
    uint16_t mapPPUWriteAddress(const uint16_t& address) const override;
};

#endif
