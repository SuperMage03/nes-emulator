#ifndef _PPU_BUS_HPP_
#define _PPU_BUS_HPP_
#include "bus.hpp"
// Project Headers
#include "rp2C02.hpp"
#include "cartridge.hpp"
#include "memory-unit.hpp"
// Project Defines
#define PPU_BUS_NAME_TABLE_SIZE 0x0800
#define PPU_BUS_PALETTE_TABLE_SIZE 0x0020

class PPUBUS : public BUS {
public:
    /**
    * @brief  Constructor for BUS
    * @param  vram: RAM for PPU (Stores Two Name Tables)
    * @param  cartridge: Cartridge on the BUS
    * @return None
    */
    PPUBUS(RP2C02& ppu, MemoryUnit& vram, const std::unique_ptr<Cartridge>& cartridge);

    /**
    * @brief  Reads data from the bus at the address
    * @param  address: The address to read from
    * @return Data read from the bus
    */
    uint8_t readBusData(const uint16_t& address) const override;
    
    /**
    * @brief  Writes data to the bus at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool writeBusData(const uint16_t& address, const uint8_t& data) override;

private:
    RP2C02& ppu_;
    MemoryUnit& vram_;
    const std::unique_ptr<Cartridge>& cartridge_;
};

#endif
