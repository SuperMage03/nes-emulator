#ifndef _NES_HPP_
#define _NES_HPP_

#include "cartridge.hpp"
#include "mos6502.hpp"
#include "memory-unit.hpp"
#include "ppu-bus.hpp"
#include "rp2C02.hpp"
#include "cpu-bus.hpp"

class NES {
public:
    /**
    * @brief  Constructor for NES
    * @param  None
    * @return None
    */
    NES();

    /**
    * @brief  Destructor for NES
    * @param  None
    * @return None
    */
    ~NES();

    /**
    * @brief  Loads the cartridge from the file path
    * @param  path: The file path to the cartridge
    * @return None
    */
    void loadCartridge(const std::string& path);

    /**
    * @brief  Releases the cartridge from the NES system
    * @param  None
    * @return None
    */
    void releaseCartridge();

    /**
    * @brief  Resets the NES system
    * @param  None
    * @return None
    */
    void reset();

    /**
    * @brief  Runs 1 clock cycle of the NES system
    * @param  None
    * @return None
    */
    void clock();

private:
    MOS6502 cpu_;
    MemoryUnit ram_;
    RP2C02 ppu_;
    MemoryUnit name_table_;
    MemoryUnit palette_table_;    
    std::unique_ptr<Cartridge> cartridge_;
    CPUBUS cpu_bus_;
    PPUBUS ppu_bus_;
};

#endif
