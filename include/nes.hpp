#ifndef _NES_HPP_
#define _NES_HPP_
// Project Headers
#include "nes-window.hpp"
#include "nes-sound.hpp"
#include "cartridge.hpp"
#include "rp2A03.hpp"
#include "cpu-bus.hpp"
#include "rp2C02.hpp"
#include "ppu-bus.hpp"
#include "controller.hpp"
#include "memory-unit.hpp"

class NES {
public:
    /**
    * @brief  Constructor for NES
    * @param  None
    * @return None
    */
    NES();

    /**
    * @brief  Connects the NES system to the display window
    * @param  window: The display window to connect to
    * @return None
    */
    void connectDisplayWindow(NESWindow& window);

    /**
    * @brief  Connects the NES system to the sound system
    * @param  sound_system: The sound system to connect to
    * @return None
    */
    void connectSoundSystem(NESSound& sound_system);

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

    /**
    * @brief  Runs 1 frame update of the NES system
    * @param  None
    * @return None
    */
    void stepFrame();

    /**
    * @brief  Connects the controller to the NES system
    * @param  controller: The controller to connect
    * @return None
    */
    void connectController(Controller& controller);

private:
    uint64_t clock_count_;
    RP2A03 cpu_;
    MemoryUnit ram_;
    RP2C02 ppu_;
    MemoryUnit vram_;
    MemoryUnit palette_table_;    
    std::unique_ptr<Cartridge> cartridge_;
    CPUBUS cpu_bus_;
    PPUBUS ppu_bus_;

    // Friending classes for access to private members
    friend class NESDebugWindow;
};

#endif
