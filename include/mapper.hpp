#ifndef _MAPPER_HPP_
#define _MAPPER_HPP_
// Standard Library Headers
#include <memory>
#include <cstdint>

class Mapper {
public:
    /*
    * @brief  Creates an instance of a Mapper based on the mapper_id
    * @param  mapper_id: The ID of the Mapper
    * @return A unique pointer to the created Mapper instance
    */
    static std::unique_ptr<Mapper> makeMapper(const uint8_t& mapper_id);

    // Destructor
    virtual ~Mapper() = default;

    /*
    * @brief  Returns the mapped address for the CPU to read from the Cartridge
    * @param  address: The address to read from
    * @return Mapped address
    */
    virtual uint16_t mapCPUReadAddress(const uint16_t& address) const = 0;

    /*
    * @brief  Returns the mapped address for the CPU to write from the Cartridge
    * @param  address: The address to write to
    * @return Mapped Address
    */
    virtual uint16_t mapCPUWriteAddress(const uint16_t& address) const = 0;

    /*
    * @brief  Returns the mapped address for the PPU to read from the Cartridge
    * @param  address: The address to read from
    * @return Mapped Address
    */
    virtual uint16_t mapPPUReadAddress(const uint16_t& address) const = 0;

    /*
    * @brief  Returns the mapped address for the PPU to write from the Cartridge
    * @param  address: The address to write to
    * @return Mapped Address
    */
    virtual uint16_t mapPPUWriteAddress(const uint16_t& address) const = 0;
};

#endif
