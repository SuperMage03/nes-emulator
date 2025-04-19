#include "rp2C02.hpp"
#include <stdexcept>

static uint8_t flipByte(uint8_t byte) {
    uint8_t result = 0x00;
    for (uint8_t i = 0; i < 8; i++) {
        result <<= 1;
        result |= (byte & 0x01);
        byte >>= 1;
    }
    return result;
}

bool RP2C02::Sprite::isFlippedHorizontally() const {
    return attribute & 0x40;
}

bool RP2C02::Sprite::isFlippedVertically() const {
    return attribute & 0x80;
}

RP2C02::RP2C02(): 
    colour_palette_({
        // Thanks to OLC
        { 84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, { 48,   0, 136},
        { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, { 60,  24,   0},
        { 32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, {  0,  60,   0},
        {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    
        {152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, { 92,  30, 228},
        {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, {120,  60,   0},
        { 84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40},
        {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
    
        {236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236},
        {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, {212, 136,  32},
        {160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108},
        { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {  0,   0,   0},
    
        {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236},
        {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
        {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
        {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {  0,   0,   0},
    }),
    palette_table_({0}), oam_({.raw_data={0}}),
    control_register_({.raw_val=0x00}),
    mask_register_({.raw_val=0x00}),
    status_register_({.raw_val=0x00}),
    oam_address_(0), oam_data_(0),
    loopy_v_register_({.raw_val=0x0000}),
    loopy_t_register_({.raw_val=0x0000}),
    fine_x_scroll_(0), is_high_byte_selected_(true),
    data_buffer_(0), read_from_data_buffer_(false),
    nmi_requested_(false), cycles_elapsed_(0), 
    scanline_(0), scanline_cycle_(0), 
    window_(nullptr), bus_(nullptr) {
}

void RP2C02::connectDisplayWindow(NESWindow* window) {
    window_ = window;
}

void RP2C02::connectBUS(BUS* target_bus) {
    bus_ = target_bus;
}

void RP2C02::runCycle() {
    // Increment on total PPU cycles elapsed
    cycles_elapsed_++;

    // Skip scanline cycle 0 for scanline 0
    if ((scanline_ == 0) && (scanline_cycle_ == 0)) {
        scanline_cycle_ = 1;
        return;
    }

    // --------------- VBlank Related Events -----------------
    // Pre-rendering scanline
    if (scanline_ == -1) {
        // Clear VBLANK flag at cycle 1
        if (scanline_cycle_ == 1) {
            status_register_.VBLANK = 0;
            status_register_.SPRITE_OVERFLOW = 0;
            // Clear the sprite shifters
            sprite_shifter_pattern_lo_.fill(0x00);
            sprite_shifter_pattern_hi_.fill(0x00);
        }
    }
    // Scanline 241 (The scanline right after the post-render scanline)
    else if (scanline_ == 241) {
        if (scanline_cycle_ == 1) {
            status_register_.VBLANK = 1;
            if (control_register_.GENERATE_NMI) {
                nmi_requested_ = true;
            }
        }
    }

    // Fetching next background tile data
    if ((-1 <= scanline_) && (scanline_ <= 239)) {
        // These cycles, we are fetching next tile data
        if (((2 <= scanline_cycle_) && (scanline_cycle_ <= 257)) ||
            ((321 <= scanline_cycle_) && (scanline_cycle_ <= 337))) {
            if (mask_register_.BACKGROUND_ENABLE) {
                // Shift the background shifters
                shiftBackgroundShifters();
            }
            
            // We fetch new tile data every 8 cycles
            switch ((scanline_cycle_ - 1) % 8) {
                case 0:
                loadBackgroundShifers(bg_next_tile_lsb_, bg_next_tile_msb_, bg_next_tile_palette_id_);
                break;
                // Fetch name table data
                case 1:
                bg_next_tile_id_ = bus_->readBusData(0x2000 | (loopy_v_register_.raw_val & 0x0FFF));
                break;
                // Fetch attribute table data
                case 3: {
                    // Here is where the loopy register shines, formula is on the NESDev Wiki
                    uint16_t attribute_address = 0x23C0 | 
                        (loopy_v_register_.NAMETABLE_Y << 11) | 
                        (loopy_v_register_.NAMETABLE_X << 10) |
                        ((loopy_v_register_.COARSE_Y >> 2) << 3) | 
                        (loopy_v_register_.COARSE_X >> 2);
                    
                    bg_next_tile_palette_id_ = bus_->readBusData(attribute_address);

                    // We extract the palette ID from the quadrant where the tile is located
                    uint8_t attribute_block_x = loopy_v_register_.COARSE_X % 4;
                    uint8_t attribute_block_y = loopy_v_register_.COARSE_Y % 4;
                    if ((attribute_block_x <= 1) && (attribute_block_y <= 1)) {
                        // Quad 0 of the attribute block
                        bg_next_tile_palette_id_ = (bg_next_tile_palette_id_ & 0b00000011);
                    }
                    else if ((attribute_block_x > 1) && (attribute_block_y <= 1)) {
                        // Quad 1 of the attribute block
                        bg_next_tile_palette_id_ = (bg_next_tile_palette_id_ & 0b00001100) >> 2;
                    }
                    else if ((attribute_block_x <= 1) && (attribute_block_y > 1)) {
                        // Quad 2 of the attribute block
                        bg_next_tile_palette_id_ = (bg_next_tile_palette_id_ & 0b00110000) >> 4;
                    }
                    else {
                        // Quad 3 of the attribute block
                        bg_next_tile_palette_id_ = (bg_next_tile_palette_id_ & 0b11000000) >> 6;
                    }
                }
                break;
                // Fetch LSB of tile data from pattern table
                case 5:
                bg_next_tile_lsb_ = bus_->readBusData(control_register_.BACKGROUND_PATTERN_TABLE * 0x1000 + bg_next_tile_id_ * 0x10 + loopy_v_register_.FINE_Y + 0);
                break;
                // Fetch MSB of tile data from pattern table and increment scroll X
                case 7:
                // Fetch MSB of tile data from pattern table
                bg_next_tile_msb_ = bus_->readBusData(control_register_.BACKGROUND_PATTERN_TABLE * 0x1000 + bg_next_tile_id_ * 0x10 + loopy_v_register_.FINE_Y + 8);
                // Increment the scroll X register
                increaseScrollX();
                break;
            }
        }

        // End of the displayable scanline cycle, move scroll Y down
        if (scanline_cycle_ == 256) {
            increaseScrollY();
        }

        // Transfers the horizontal scroll registers
        if (scanline_cycle_ == 257) {
            // Don't think this is needed
            // loadBackgroundShifers(bg_next_tile_lsb_, bg_next_tile_msb_, bg_next_tile_palette_id_);
            transferLoopyX();
        }

        // Weird unnecessary tile read at the end of the scanline
        if ((scanline_cycle_ == 338) || (scanline_cycle_ == 340)) {
            bg_next_tile_id_ = bus_->readBusData(0x2000 | (loopy_v_register_.raw_val & 0x0FFF));
        }

        // Pre-render scanline
        if (scanline_ == -1) {
            if ((280 <= scanline_cycle_) && (scanline_cycle_ <= 304)) {
                transferLoopyY();
            }
        }
    }

    uint8_t bg_pixel_colour_value = 0x00;
    uint8_t bg_palette_id = 0x00;

    if (mask_register_.BACKGROUND_ENABLE) {
        uint16_t scroll_x_mask = 0x8000 >> fine_x_scroll_;

        // Gets the pixel colour value from the background shifters
        bg_pixel_colour_value |= (bg_shifter_pattern_hi_ & scroll_x_mask) > 0;
        bg_pixel_colour_value <<= 1;
        bg_pixel_colour_value |= (bg_shifter_pattern_lo_ & scroll_x_mask) > 0;

        // Gets the palette ID from the background shifters
        bg_palette_id |= (bg_shifter_palette_hi_ & scroll_x_mask) > 0;
        bg_palette_id <<= 1;
        bg_palette_id |= (bg_shifter_palette_lo_ & scroll_x_mask) > 0;
    }

    // --------------- Code block for rendering the sprites --------------------

    if ((0 <= scanline_) && (scanline_ <= 239)) {
        // Find the sprites at the next scanline
        if (scanline_cycle_ == 257) {
            sprites_at_next_scanline_ = std::move(searchSpritesAtScanline(scanline_));
            // If there are more than 8 sprites on the next scanline, set the overflow flag
            status_register_.SPRITE_OVERFLOW = (sprites_at_next_scanline_.size() > 8);
        }

        // Load the sprites at the next scanline into the sprite shifters
        if (scanline_cycle_ == 340) {
            for (uint8_t sprite_index = 0; sprite_index < std::min(sprites_at_next_scanline_.size(), static_cast<size_t>(8)); sprite_index++) {
                const auto& sprite = sprites_at_next_scanline_.at(sprite_index);

                // The y coordinate within the sprite to be rendered
                uint8_t sprite_pixel_y = scanline_ - sprite.y_position;

                // In 8x16 sprite mode, the pattern table to use is the last bit of the tile ID
                uint8_t sprite_pattern_table = control_register_.SPRITE_SIZE ? sprite.tile_id & 0x01 : control_register_.SPRITE_PATTERN_TABLE;
                // In 8x16 sprite mode, the upper 7 bit of the tile ID is for the first half, and the other half of the sprite is the next tile
                uint8_t sprite_tile_id = control_register_.SPRITE_SIZE ? (sprite.tile_id & 0xFE) + (sprite_pixel_y >= 8) : sprite.tile_id;

                // The y coordinate within the tile to be rendered
                uint8_t tile_pixel_y = sprite_pixel_y % 8;

                // If the sprite is flipped vertically, we need to flip the tile pixel y coordinate
                tile_pixel_y = sprite.isFlippedVertically() ? 7 - tile_pixel_y : tile_pixel_y;

                // If the sprite is flipped vertically, and we are in 8x16 sprite mode, we need to render the other tile of the sprite
                //   Simply done by toggling the last bit of the tile ID
                if (sprite.isFlippedVertically() && control_register_.SPRITE_SIZE) {
                    sprite_tile_id ^= 0x01;
                }

                // Read the tile data from the pattern table
                uint8_t tile_pattern_lsb = bus_->readBusData(sprite_pattern_table * 0x1000 + sprite_tile_id * 0x10 + tile_pixel_y + 0);
                uint8_t tile_pattern_msb = bus_->readBusData(sprite_pattern_table * 0x1000 + sprite_tile_id * 0x10 + tile_pixel_y + 8);

                if (sprite.isFlippedHorizontally()) {
                    tile_pattern_lsb = flipByte(tile_pattern_lsb);
                    tile_pattern_msb = flipByte(tile_pattern_msb);
                }

                loadSpriteShifters(tile_pattern_lsb, tile_pattern_msb, sprite_index);
            }
        }
    }

    uint8_t sprite_pixel_colour_value = 0x00;
    uint8_t sprite_palette_id = 0x00;
    uint8_t sprite_z_index = 0x00;

    if (mask_register_.SPRITE_ENABLE) {
        for (uint8_t sprite_index = 0; sprite_index < std::min(sprites_at_next_scanline_.size(), static_cast<size_t>(8)); sprite_index++) {
            Sprite& sprite = sprites_at_next_scanline_.at(sprite_index);
            // Scanline cycle is not at the position to draw the sprite yet
            if (sprite.x_position > 0) {
                continue;
            }
            
            // Gets the pixel colour value from the sprite shifters
            sprite_pixel_colour_value |= (sprite_shifter_pattern_hi_.at(sprite_index) & 0x80) > 0;
            sprite_pixel_colour_value <<= 1;
            sprite_pixel_colour_value |= (sprite_shifter_pattern_lo_.at(sprite_index) & 0x80) > 0;
            
            sprite_palette_id = (sprite.attribute & 0x03) + 0x04;
            sprite_z_index = (sprite.attribute & 0x20) == 0;

            // Check that this pixel is not transparent
            if (sprite_pixel_colour_value != 0x00) break;
        }

        // Do sprite shifting
        if ((0 <= scanline_) && (scanline_ <= 239) &&
            (0 <= (scanline_cycle_ - 1) && ((scanline_cycle_ - 1) <= 255))) {
            for (uint8_t sprite_index = 0; sprite_index < std::min(sprites_at_next_scanline_.size(), static_cast<size_t>(8)); sprite_index++) {
                Sprite& sprite = sprites_at_next_scanline_.at(sprite_index);
                // Waiting until the sprite x position lines up with scanline cycle
                if (sprite.x_position > 0) {
                    sprite.x_position--;
                    continue;
                }
                shiftSpriteShifters(sprite_index);
            }
        }
    }

    uint8_t final_pixel_colour_value = 0x00;
    uint8_t final_palette_id = 0x00;

    // Both background and sprite are transparent
    if ((bg_pixel_colour_value == 0x00) && (sprite_pixel_colour_value == 0x00)) {
        final_pixel_colour_value = 0x00;
        final_palette_id = 0x00;
    }
    else if ((bg_pixel_colour_value != 0x00) && (sprite_pixel_colour_value == 0x00)) {
        final_pixel_colour_value = bg_pixel_colour_value;
        final_palette_id = bg_palette_id;
    }
    else if ((bg_pixel_colour_value == 0x00) && (sprite_pixel_colour_value != 0x00)) {
        final_pixel_colour_value = sprite_pixel_colour_value;
        final_palette_id = sprite_palette_id;
    }
    else {
        final_pixel_colour_value = sprite_z_index > 0 ? sprite_pixel_colour_value : bg_pixel_colour_value;
        final_palette_id = sprite_z_index > 0 ? sprite_palette_id : bg_palette_id;
    }

    if (window_ != nullptr) {
        window_->setPixel(scanline_cycle_ - 1, scanline_, getColourFromPalette(final_palette_id, final_pixel_colour_value));
    }

    // Scanline and Scanline Cycle Increment
    scanline_cycle_++;
    if (scanline_cycle_ >= 341) {
        scanline_cycle_ = 0;
        if (scanline_ == 260) {
            // Increment the scanline to pre-render scanline
            scanline_ = -1;
        }
        else {
            scanline_++;
        }
    }
}

uint8_t RP2C02::readPaletteTable(const uint16_t& address) const {
    return palette_table_.at(address);
}

bool RP2C02::writePaletteTable(const uint16_t& address, const uint8_t& data) {
    try {
        palette_table_.at(address) = data;
        return true;
    } catch (std::out_of_range& err) {
        return false;
    }
}

uint8_t RP2C02::readRegister(const uint8_t& address) {
    uint8_t return_value = 0x00;

    switch (address & 0b00000111) {
        case 0x00:
            return_value = control_register_.raw_val;
            break;
        case 0x01:
            return_value = mask_register_.raw_val;
            break;
        case 0x02:
            return_value = (status_register_.raw_val & 0xE0) | (data_buffer_ & 0x1F);
            // Clear the vertical blanking flag
            status_register_.VBLANK = 0;
            // Reset byte select variable
            is_high_byte_selected_ = true;
            break;
        case 0x04:
            return_value = oam_.raw_data.at(oam_address_);
            break;
        case 0x07: {
            return_value = data_buffer_;

            // Read VRAM data from address
            data_buffer_ = bus_->readBusData(loopy_v_register_.raw_val);

            // If we can immediately return read data
            if (!read_from_data_buffer_) {
                return_value = data_buffer_;
            }
            // Increment address register
            loopy_v_register_.raw_val += control_register_.VRAM_ADDRESS_INCREMENT_MODE ? 0x20 : 0x01;
            break;
        }
        default:
            break;
    }
    return return_value;
}

bool RP2C02::writeRegister(const uint8_t& address, const uint8_t& data) {
    bool write_success = false;

    switch (address & 0b00000111) {
        case 0x00:
            control_register_.raw_val = data;
            loopy_t_register_.NAMETABLE_X = control_register_.NAMETABLE_X;
            loopy_t_register_.NAMETABLE_Y = control_register_.NAMETABLE_Y;
            write_success = true;
            break;
        case 0x01:
            mask_register_.raw_val = data;
            write_success = true;
            break;
        case 0x03:
            oam_address_ = data;
            write_success = true;
            break;
        case 0x04:
            oam_.raw_data.at(oam_address_) = data;
            oam_address_++;
            write_success = true;
            break;
        case 0x05: {
            if (is_high_byte_selected_) {
                fine_x_scroll_ = data & 0b00000111;
                loopy_t_register_.COARSE_X = data >> 3;
                is_high_byte_selected_ = false;
            }
            else {
                loopy_t_register_.FINE_Y = data & 0b00000111;
                loopy_t_register_.COARSE_Y = data >> 3;
                is_high_byte_selected_ = true;
            }
            write_success = true;
            break;
        }
        case 0x06: {
            if (is_high_byte_selected_) {
                loopy_t_register_.HI = data;
                is_high_byte_selected_ = false;
            }
            else {
                loopy_t_register_.LO = data;
                // Set the VRAM address to the temporary address
                loopy_v_register_.raw_val = loopy_t_register_.raw_val;
                is_high_byte_selected_ = true;
            }
            write_success = true;
            break;
        }
        case 0x07: {
            write_success = bus_->writeBusData(loopy_v_register_.raw_val, data);
            loopy_v_register_.raw_val += control_register_.VRAM_ADDRESS_INCREMENT_MODE ? 0x20 : 0x01;
            break;
        }
        default:
            break;
    }

    return write_success;
}

void RP2C02::setReadFromDataBuffer(const bool& value) {
    read_from_data_buffer_ = value;
}

bool RP2C02::getNMIFlag() const {
    return nmi_requested_;
}

void RP2C02::setNMIFlag(const bool& value) {
    nmi_requested_ = value;
}

bool RP2C02::isRendering() const {
    return (mask_register_.BACKGROUND_ENABLE || mask_register_.SPRITE_ENABLE);
}

void RP2C02::increaseScrollX() {
    if (!isRendering()) return;

    if (loopy_v_register_.COARSE_X == 31) {
        loopy_v_register_.COARSE_X = 0;
        loopy_v_register_.NAMETABLE_X = ~loopy_v_register_.NAMETABLE_X;
    }
    else {
        loopy_v_register_.COARSE_X++;
    }
}

void RP2C02::increaseScrollY() {
    if (!isRendering()) return;

    // Increment the fine Y coordinate (1 pixel down within the tile)
    if (loopy_v_register_.FINE_Y < 7) {
        loopy_v_register_.FINE_Y++;
    }
    else {
        loopy_v_register_.FINE_Y = 0;
        // Increment the coarse Y coordinate (1 tile down within the name table)
        if (loopy_v_register_.COARSE_Y == 29) {
            loopy_v_register_.COARSE_Y = 0;
            loopy_v_register_.NAMETABLE_Y = ~loopy_v_register_.NAMETABLE_Y;
        }
        // If we are ever in the last row of the name table (which we shouldn't since 30 is the last row of the displayable name table),
        //   just wrap around to the first row
        else if (loopy_v_register_.COARSE_Y == 31) {
            loopy_v_register_.COARSE_Y = 0;
        }
        else {
            loopy_v_register_.COARSE_Y++;
        }
    }
}

void RP2C02::transferLoopyX() {
    if (!isRendering()) return;
    loopy_v_register_.NAMETABLE_X = loopy_t_register_.NAMETABLE_X;
    loopy_v_register_.COARSE_X = loopy_t_register_.COARSE_X;
}

void RP2C02::transferLoopyY() {
    if (!isRendering()) return;
    loopy_v_register_.NAMETABLE_Y = loopy_t_register_.NAMETABLE_Y;
    loopy_v_register_.COARSE_Y = loopy_t_register_.COARSE_Y;
    loopy_v_register_.FINE_Y = loopy_t_register_.FINE_Y;
}

void RP2C02::loadBackgroundShifers(const uint8_t& tile_pattern_lsb, const uint8_t& tile_pattern_msb, const uint8_t& palette_id) {
    // Clear all lower bytes of the shifters
    bg_shifter_pattern_lo_ &= 0xFF00;
    bg_shifter_pattern_hi_ &= 0xFF00;
    bg_shifter_palette_lo_ &= 0xFF00;
    bg_shifter_palette_hi_ &= 0xFF00;
    // Push the new tile data into the shifters
    bg_shifter_pattern_lo_ |= tile_pattern_lsb;
    bg_shifter_pattern_hi_ |= tile_pattern_msb;
    if (palette_id & 0b00000001) {
        bg_shifter_palette_lo_ |= 0xFF;
    }
    if (palette_id & 0b00000010) {
        bg_shifter_palette_hi_ |= 0xFF;
    }
}

void RP2C02::shiftBackgroundShifters() {
    bg_shifter_pattern_lo_ <<= 1;
    bg_shifter_pattern_hi_ <<= 1;
    bg_shifter_palette_lo_ <<= 1;
    bg_shifter_palette_hi_ <<= 1;
}

void RP2C02::loadSpriteShifters(const uint8_t& tile_pattern_lsb, const uint8_t& tile_pattern_msb, const uint8_t& sprite_index) {
    // Push the new tile data into the shifters
    sprite_shifter_pattern_lo_.at(sprite_index) = tile_pattern_lsb;
    sprite_shifter_pattern_hi_.at(sprite_index) = tile_pattern_msb;
}

void RP2C02::shiftSpriteShifters(const uint8_t& sprite_index) {
    sprite_shifter_pattern_lo_.at(sprite_index) <<= 1;
    sprite_shifter_pattern_hi_.at(sprite_index) <<= 1;
}

NESWindow::Colour RP2C02::getColourFromPalette(const uint8_t& palette_id, const uint8_t& pixel_colour_value) const {
    return colour_palette_.at(bus_->readBusData(0x3F00 + ((palette_id << 2) | pixel_colour_value)) % colour_palette_.size());
}

RP2C02::Tile RP2C02::getTileFromPatternTable(const uint8_t& tile_index, const uint8_t& palette_id, const uint8_t& pattern_table_index) const {
    const unsigned int total_bytes_per_tile = 16; // 8x8 pixels, 2 bits per pixel
    const unsigned int tile_pixel_per_side = 8;
    const unsigned int chr_rom_pattern_table_byte_size = 0x1000;

    const uint16_t flattened_tile_byte_index = tile_index * total_bytes_per_tile;

    Tile out;
    for (uint8_t pixel_y = 0; pixel_y < tile_pixel_per_side; pixel_y++) {
        uint8_t cur_row_pixel_lsb = bus_->readBusData(pattern_table_index * chr_rom_pattern_table_byte_size + flattened_tile_byte_index + pixel_y + 0);
        uint8_t cur_row_pixel_msb = bus_->readBusData(pattern_table_index * chr_rom_pattern_table_byte_size + flattened_tile_byte_index + pixel_y + 8);

        for (int8_t pixel_x = tile_pixel_per_side - 1; pixel_x >= 0; pixel_x--) {
            uint8_t pixel_colour_value = (cur_row_pixel_msb & 0x01) << 1 | (cur_row_pixel_lsb & 0x01);
            cur_row_pixel_msb >>= 1;
            cur_row_pixel_lsb >>= 1;

            NESWindow::Colour result_colour = getColourFromPalette(palette_id, pixel_colour_value);
            out.pixel_colour.at(pixel_y * tile_pixel_per_side + pixel_x) = result_colour;
        }
    }
    return out;
}

const RP2C02::OAM& RP2C02::getOAM() const {
    return oam_;
}

std::vector<RP2C02::Sprite> RP2C02::searchSpritesAtScanline(const int16_t& scanline) const {
    std::vector<Sprite> sprites_found;
    for (const auto& sprite : oam_.sprite_data) {
        if ((sprite.y_position <= scanline) && (scanline < (sprite.y_position + (control_register_.SPRITE_SIZE ? 16 : 8)))) {
            sprites_found.push_back(sprite);
        }
    }
    return sprites_found;
}
