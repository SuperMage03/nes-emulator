#ifndef _CPU_BUS_HPP_
#define _CPU_BUS_HPP_
// Project Headers
#include "bus.hpp"
#include "rp2A03.hpp"
#include "rp2C02.hpp"
#include "cartridge.hpp"
#include "controller.hpp"
#include "memory-unit.hpp"
// Project Defines
#define CPU_BUS_RAM_SIZE 0x0800
#define CPU_BUS_PPU_SIZE 0x0008

class CPUBUS : public BUS {
public:
    /**
    * @brief  Constructor for BUS
    * @param  cpu: CPU on the BUS
    * @param  ram: RAM on the BUS
    * @param  ppu: PPU on the BUS
    * @param  cartridge: Cartridge on the BUS
    * @return None
    */
    CPUBUS(RP2A03& cpu, MemoryUnit& ram, RP2C02& ppu, const std::unique_ptr<Cartridge>& cartridge);

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

    /**
    * @brief  Connects the controller to the bus
    * @param  controller: The controller to connect
    * @return True if successfully connected, false if two controllers are already connected
    */
    bool connectController(Controller* controller);

private:
    RP2A03& cpu_;
    MemoryUnit& ram_;
    RP2C02& ppu_;
    const std::unique_ptr<Cartridge>& cartridge_;
    std::array<Controller*, 2> controllers_;
};

#endif
