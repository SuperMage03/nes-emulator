#ifndef _BUS_HPP_
#define _BUS_HPP_
// Stardard Library Headers
#include <cstdint>

class BUS {
public:
    // Destructor
    virtual ~BUS() = default;

    /**
    * @brief  Reads data from the bus at the address
    * @param  address: The address to read from
    * @return Data read from the bus
    */
    virtual uint8_t readBusData(const uint16_t& address) const = 0;
    
    /**
    * @brief  Writes data to the bus at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    virtual bool writeBusData(const uint16_t& address, const uint8_t& data) = 0;
};

#endif
