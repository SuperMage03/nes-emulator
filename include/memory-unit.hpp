#ifndef _MEMORY_UNIT_HPP_
#define _MEMORY_UNIT_HPP_
// Stardard Library Headers
// #include <fstream>
#include <memory>
#include <cstdint>

class MemoryUnit {
public:
    /**
    * @brief  Constructor for RAM
    * @param  byte_size: The size of the RAM in bytes
    * @return None
    */
    MemoryUnit(const uint32_t& byte_size);

    /**
    * @brief  Reads 1 byte of data at given memory address
    * @param  address: The memory address to read
    * @return Data read at address
    */
    uint8_t read(const uint16_t& address) const;

    /**
    * @brief  Writes 1 byte of data at given memory address
    * @param  address: The memory address to write
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool write(const uint16_t& address, const uint8_t& data) const;

    /**
    * @brief  Gets the pointer to memory block array
    * @param  None
    * @return The memory block array
    */
    uint8_t* getPointer() const;

    /**
    * @brief  Gets the size of the memory block
    * @param  None
    * @return The size of the memory block
    */
    uint32_t getSize() const;

private:
    const uint32_t byte_size_;
    std::unique_ptr<uint8_t[]> memory_block_;
};

#endif
