#ifndef _RP2CO2_HPP_
#define _RP2CO2_HPP_
// Standard Library Headers
#include <cstdint>
// Project Headers
#include "nes-window.hpp"
#include "bus.hpp"

class RP2C02 {
public:
    /**
    * @brief  Connects PPU to Display Window
    * @param  window: The NES Window to connect to
    * @return None
    */
    void connectDisplayWindow(NESWindow* window);

    /**
    * @brief  Connects PPU to BUS
    * @param  None
    * @return None
    */
    void connectBUS(BUS* target_bus);

    /**
    * @brief  Run 1 cycle of the PPU
    * @param  None
    * @return None
    */
    void runCycle();

    /**
    * @brief  Reads data from the PPU at the address
    * @param  address: The address to read from
    * @return Data read from the PPU
    */
    uint8_t read(const uint16_t& address) const;
    
    /**
    * @brief  Writes data to the PPU at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool write(const uint16_t& address, const uint8_t& data);

private:
    uint64_t cycles_elapsed_;
    uint16_t drawing_row;
    uint16_t drawing_col;
    NESWindow* window_;
    BUS* bus_;
};

#endif
