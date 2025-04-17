#ifndef _RP2CO2_HPP_
#define _RP2CO2_HPP_
// Standard Library Headers
#include <array>
#include <cstdint>
// Project Headers
#include "nes-window.hpp"
#include "bus.hpp"

class RP2C02 {
public:
    union ControlRegister {
        struct {
            uint8_t NAMETABLE : 2;
            uint8_t VRAM_ADDRESS_INCREMENT_MODE : 1;
            uint8_t SPRITE_PATTERN_ADDRESS : 1;
            uint8_t BACKGROUND_PATTERN_ADDRESS : 1;
            uint8_t SPRITE_SIZE : 1;
            uint8_t MASTER_SLAVE_SELECT : 1;
            uint8_t GENERATE_NMI : 1;
        };
        uint8_t raw_val;
    };

    union MaskRegister {
        struct {
            uint8_t GREYSCALE : 1;
            uint8_t BACKGROUND_LEFT_COL_ENABLE : 1;
            uint8_t SPRITE_LEFT_COL_ENABLE : 1;
            uint8_t BACKGROUND_ENABLE : 1;
            uint8_t SPRITE_ENABLE : 1;
            uint8_t COLOUR_EMPHASIS : 3;
        };
        uint8_t raw_val;
    };

    union StatusRegister {
        struct {
            uint8_t UNUSED : 5;
            uint8_t SPRITE_OVERFLOW : 1;
            uint8_t SPRITE_ZERO_HIT : 1;
            uint8_t VBLANK : 1;
        };
        uint8_t raw_val;
    };

    union ScrollRegister {
        struct {
            uint8_t Y;
            uint8_t X;
        };
        uint16_t ral_val;
    };

    union NameTable {
        struct {
            std::array<uint8_t, 0x03C0> tile_data;
            std::array<uint8_t, 0x0040> palette_data;
        };
        std::array<uint8_t, 0x0400> raw_data;
    };

    struct Tile {
        std::array<NESWindow::Colour, 0x40> pixel_colour;
    };

    // Constructor
    RP2C02();

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
    * @brief  Reads data from the Palette Table at the address
    * @param  address: The address to read from
    * @return Data read from the PPU
    */
    uint8_t readPaletteTable(const uint16_t& address) const;
    
    /**
    * @brief  Writes data to the Palette Table at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool writePaletteTable(const uint16_t& address, const uint8_t& data);

    /**
    * @brief  Reads data from the PPU register at the address
    * @param  address: The address to read from
    * @return Data read from the PPU
    */
    uint8_t readRegister(const uint8_t& address);
    
    /**
    * @brief  Writes data to the PPU register at the address
    * @param  address: The address to write to
    * @param  data: The data to write
    * @return True if successfully written, false otherwise
    */
    bool writeRegister(const uint8_t& address, const uint8_t& data);

    /**
    * @brief  Setter for read_from_data_buffer_
    * @param  value: new value
    * @return None
    */
    void setReadFromDataBuffer(const bool& value);

    /**
    * @brief  Getter for nmi_requested_
    * @param  None
    * @return nmi_requested_
    */
    bool getNMIFlag() const;

    /**
    * @brief  Setter for nmi_requested_
    * @param  value: new value
    * @return None
    */
    void setNMIFlag(const bool& value);

    /**
    * @brief  Gets the colour from the palette
    * @param  palette_id: The ID of the palette
    * @param  pixel_colour_value: The pixel colour value
    * @return Colour from the palette
    */
    NESWindow::Colour getColourFromPalette(const uint8_t& palette_id, const uint8_t& pixel_colour_value) const;

    /**
    * @brief  Gets the tile from the pattern table
    * @param  tile_index: The index of the tile
    * @param  pattern_table_index: The index of the pattern table
    * @param  palette_id: The ID of the palette
    * @return Tile from the pattern table
    */
    Tile getTileFromPatternTable(const uint8_t& tile_index, const uint8_t& pattern_table_index, const uint8_t& palette_id) const;

    /**
    * @brief  Gets mirrored vram address
    * @param  address: The address to be mirrored
    * @return Mirrored address
    */
    uint16_t mirrorVRAMAddress(const uint16_t& address) const;

private:
    // Colour Palette for display
    std::array<NESWindow::Colour, 0x40> colour_palette_;
    // Palette Table (Keeps the palette table used on screen)
    std::array<uint8_t, 0x20> palette_table_;
    // OAM (Keeps the state of the sprites)
    std::array<uint8_t, 0x0100> oam_;

    // ------------ Internal PPU Registers ------------
    // 0x0000 (Internally) -> 0x2000 (CPU Address)
    ControlRegister control_register_;
    // 0x0001 (Internally) -> 0x2001 (CPU Address)
    MaskRegister mask_register_;
    // 0x0002 (Internally) -> 0x2002 (CPU Address)
    StatusRegister status_register_;
    // 0x0003 (Internally) -> 0x2003 (CPU Address)
    uint8_t oam_address_;
    // 0x0004 (Internally) -> 0x2004 (CPU Address)
    uint8_t oam_data_;
    // 0x0005 (Internally) -> 0x2005 (CPU Address)
    ScrollRegister scroll_register_;
    // 0x0006 (Internally) -> 0x2006 (CPU Address)
    uint16_t address_register_;
    // 0x0007 (Internally) -> 0x2007 (CPU Address)
    uint8_t data_buffer_;

    // Internal Helper Variables
    bool read_from_data_buffer_;
    bool is_high_byte_selected_;
    
    // PPU Emulator Variables
    bool nmi_requested_;
    uint64_t cycles_elapsed_;

    // Current Scanline
    uint16_t scanline_;
    // Current Scanline Cycle Count
    uint16_t cur_scanline_cycle_count_;

    // PPU External Component Pointers
    NESWindow* window_;
    BUS* bus_;
};

#endif
