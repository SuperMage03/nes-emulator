#ifndef _RP2CO2_HPP_
#define _RP2CO2_HPP_
// Standard Library Headers
#include <array>
#include <vector>
#include <cstdint>
// Project Headers
#include "nes-window.hpp"
#include "bus.hpp"

class RP2C02 {
public:
    // Thanks to the well known NES Emulator Dev Loopy for figuring out these registers
    //   It makes implementing PPU timing much easier
    union LoopyRegister {
        struct {
            uint16_t COARSE_X : 5;
            uint16_t COARSE_Y : 5;
            uint16_t NAMETABLE_X : 1;
            uint16_t NAMETABLE_Y : 1;
            uint16_t FINE_Y : 3;
            uint16_t UNUSED : 1;
        };
        struct {
            uint8_t LO;
            uint8_t HI;
        };
        uint16_t raw_val;
    };

    union ControlRegister {
        struct {
            uint8_t NAMETABLE_X : 1;
            uint8_t NAMETABLE_Y : 1;
            uint8_t VRAM_ADDRESS_INCREMENT_MODE : 1;
            uint8_t SPRITE_PATTERN_TABLE : 1;
            uint8_t BACKGROUND_PATTERN_TABLE : 1;
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

    struct Sprite {
        uint8_t y_position;
        uint8_t tile_id;
        uint8_t attribute; // How sprite should be rendered
        uint8_t x_position;
        bool isFlippedHorizontally() const;
        bool isFlippedVertically() const;
    };

    union OAM {
        std::array<Sprite, 0x40> sprite_data;
        std::array<uint8_t, 0x100> raw_data;
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
    * @brief  Returns whether the PPU is rendering or not
    * @param  None
    * @return True if rendering is enabled, false otherwise
    */
    bool isRenderEnabled() const;

    /**
    * @brief  Returns whether the PPU is rendering left-most 8 pixels or not
    * @param  None
    * @return True if rendering is enabled for left-most 8 pixels, false otherwise
    */
    bool isRenderLeftMostPixelsEnabled() const;

    /**
    * @brief  Returns whether the PPU is rendering pixel at the current cycle
    * @param  None
    * @return True if rendering pixel in the current cycle, false otherwise
    */
    bool isRenderingPixel() const;

    /**
    * @brief  Increments the scroll X coordinate
    * @param  None
    * @return None
    */
    void increaseScrollX();

    /**
    * @brief  Increments the scroll Y coordinate
    * @param  None
    * @return None
    */
    void increaseScrollY();

    /**
    * @brief  Transfers Name Table X and Coarse X from LoopyT to LoopyV
    * @param  None
    * @return None
    */
    void transferLoopyX();

    /**
    * @brief  Transfers Name Table Y, Coarse Y and Fine Y from LoopyT to LoopyV
    * @param  None
    * @return None
    */
    void transferLoopyY();

    /**
    * @brief  Loads data to the background shifters' lower byte
    * @param  tile_lsb: The LSB of the tile pattern
    * @param  tile_msb: The MSB of the tile pattern
    * @param  palette_id: The ID of the palette for the tile
    * @return None
    */
    void loadBackgroundShifers(const uint8_t& tile_pattern_lsb, const uint8_t& tile_pattern_msb, const uint8_t& palette_id);

    /**
    * @brief  Shifts background shifters to the left
    * @param  None
    * @return None
    */
    void shiftBackgroundShifters();

    /**
    * @brief  Loads data to the sprite shifters' lower byte
    * @param  tile_lsb: The LSB of the tile pattern
    * @param  tile_msb: The MSB of the tile pattern
    * @param  sprite_index: The index of the sprite
    * @return None
    */
    void loadSpriteShifters(const uint8_t& tile_pattern_lsb, const uint8_t& tile_pattern_msb, const uint8_t& sprite_index);

    /**
    * @brief  Shifts sprite shifters to the left
    * @param  sprite_index: The index of the sprite
    * @return None
    */
    void shiftSpriteShifters(const uint8_t& sprite_index);

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
    * @param  palette_id: The ID of the palette
    * @param  pattern_table_index: The index of the pattern table
    * @return Tile from the pattern table
    */
    Tile getTileFromPatternTable(const uint8_t& tile_index, const uint8_t& palette_id, const uint8_t& pattern_table_index) const;

    /**
    * @brief  Gets the current state of the OAM
    * @param  None
    * @return OAM object containing the current state of the OAM
    */
    const OAM& getOAM() const;

    /**
    * @brief  Searches for sprites at a specific scanline, then save the search result to sprites_at_next_scanline_ and assign is_sprite_zero_in_next_scanline_
    * @param  scanline: The scanline to search for
    * @return A vector of sprites that are at the specified scanline, and a bool indicating if sprite zero was part found
    */
    void searchSpritesAtScanline(const int16_t& scanline);

private:
    // Colour Palette for display
    std::array<NESWindow::Colour, 0x40> colour_palette_;
    // Palette Table (Keeps the palette table used on screen)
    std::array<uint8_t, 0x20> palette_table_;
    // OAM (Keeps the state of the sprites)
    OAM oam_;

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
    // Current VRAM address
    LoopyRegister loopy_v_register_;
    // Temporary VRAM address; Can also be thought of as the address of the top left onscreen tile.
    LoopyRegister loopy_t_register_;
    // Scroll register for the x coordinate within the tile
    uint8_t fine_x_scroll_;
    // Write toggle flag for high byte and low byte
    bool is_high_byte_selected_;

    // Data buffer register for temporary storage
    uint8_t data_buffer_;

    // Internal Helper Variables
    bool read_from_data_buffer_;
    
    // PPU Emulator Variables
    bool nmi_requested_;
    uint64_t cycles_elapsed_;

    // Current Scanline
    int16_t scanline_;
    // Current Scanline Cycle
    int16_t scanline_cycle_;

    // Next Tile ID of the background
    uint8_t bg_next_tile_id_;
    // Next Tile Attribute of the background
    uint8_t bg_next_tile_palette_id_;
    // Next Tile LSB of the background
    uint8_t bg_next_tile_lsb_;
    // Next Tile MSB of the background
    uint8_t bg_next_tile_msb_;

    // These are streams of data that are shifted out to the screen to produce the final image
    //   When bits are stacked on top of another, it will give us the selected colour
    //   Data are being pushed into these registers from the next background tile informations
    uint16_t bg_shifter_pattern_lo_;
    uint16_t bg_shifter_pattern_hi_;
    uint16_t bg_shifter_palette_lo_;
    uint16_t bg_shifter_palette_hi_;

    bool is_sprite_zero_in_next_scanline_;
    std::vector<Sprite> sprites_at_next_scanline_;
    std::array<uint8_t, 8> sprite_shifter_pattern_lo_;
    std::array<uint8_t, 8> sprite_shifter_pattern_hi_;

    // PPU External Component Pointers
    NESWindow* window_;
    BUS* bus_;
};

#endif
