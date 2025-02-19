#ifndef _RP2CO2_HPP_
#define _RP2CO2_HPP_
// Standard Library Headers
#include <cstdint>
// Project Headers
#include "bus.hpp"

class RP2C02 {
public:
    /**
    * @brief  Connects PPU to BUS
    * @param  None
    * @return None
    */
    void connectBUS(BUS* target_bus);

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
    BUS* bus_;

};

#endif
