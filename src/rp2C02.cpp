#include "rp2C02.hpp"
#include <stdexcept>

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
    nmi_requested_(false), cycles_elapsed_(0), 
    scanline_(0), cur_scanline_cycle_count_(0), 
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

    if (window_ != nullptr) {
        // window_->setPixel(cur_scanline_cycle_count_, scanline_, rand() % 256, rand() % 256, rand() % 256);
        // const Colour& cur_pixel_colour = pattern_table_cache_[1].at(cycles_elapsed_ % RP2C02_PATTERN_TABLE_PIXEL_COUNT);
        // window_->setPixel((cycles_elapsed_ % RP2C02_PATTERN_TABLE_PIXEL_COUNT) % 128, (cycles_elapsed_ % RP2C02_PATTERN_TABLE_PIXEL_COUNT) / 128, cur_pixel_colour.r, cur_pixel_colour.g, cur_pixel_colour.b);
        
        uint8_t drawing_pixel_y = (cycles_elapsed_ % 61440) / 256;
        uint8_t drawing_pixel_x = (cycles_elapsed_ % 61440) % 256;

        uint8_t tile_y = drawing_pixel_y / 8;
        uint8_t tile_x = drawing_pixel_x / 8;
        uint8_t pixel_y = drawing_pixel_y % 8;
        uint8_t pixel_x = drawing_pixel_x % 8;
        const unsigned int name_table_tile_index = tile_y * 32 + tile_x;
        const uint8_t tile_index = bus_->readBusData(0x2000 + name_table_tile_index);

        Tile tile;
        getTile(tile_index, 0, 0, tile);
        window_->setPixel(drawing_pixel_x, drawing_pixel_y, tile.pixel_colour.at(pixel_y * 8 + pixel_x).r, tile.pixel_colour.at(pixel_y * 8 + pixel_x).g, tile.pixel_colour.at(pixel_y * 8 + pixel_x).b);
    }

    cur_scanline_cycle_count_++;
    if (cur_scanline_cycle_count_ >= 341) {
        cur_scanline_cycle_count_ = 0;
        scanline_++;

        if (scanline_ == 241) {
            // Ending the frame, we are in VBLANK
            status_register_.VBLANK = 1;
            if (control_register_.GENERATE_NMI) {
                // Trigger NMI
                nmi_requested_ = true;
            }
        }

        if (scanline_ >= 262) {
            scanline_ = 0;
            // Resetting the VBLANK and start a new frame
            status_register_.VBLANK = 0;
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

uint8_t RP2C02::readRegister(const uint8_t& address, const bool& chip_select_signal) {
    uint8_t return_value = 0;
    switch (address & 0b00000111) {
        case 0x02:
            return_value = status_register_.raw_val;
        case 0x04:
            if (!chip_select_signal) {
                return_value = oam_data_;
            }
        case 0x07: {
            // Read VRAM data from address
            uint8_t read_data = bus_->readBusData(address_register_.address);
        
            return_value = data_buffer_;
            // If we can immediately return read data
            if (!read_from_data_buffer_) {
                return_value = read_data;
            }
            // Populate new cache
            data_buffer_ = read_data;
        }
    }

    // Increase register address
    if (!control_register_.VRAM_ADDRESS_INCREMENT) {
        address_register_.address += 0x01;
    }
    else {
        address_register_.address += 0x20;
    }
    return return_value;
}

bool RP2C02::writeRegister(const uint8_t& address, const bool& chip_select_signal, const uint8_t& data) {
    switch (address & 0b00000111) {
        case 0x00:
            control_register_.raw_val = data;
            return true;
        case 0x01:
            mask_register_.raw_val = data;
            return true;
        case 0x03:
            oam_address_ = data;
            return true;
        case 0x04:
            if (!chip_select_signal) {
                // OAM DATA
                return false;
            }
            oam_dma_ = data;
            return true;
        case 0x05: {
            if (scroll_register_.is_x_byte_selected) {
                scroll_register_.X = data;
            }
            else {
                scroll_register_.Y = data;
            }
            scroll_register_.is_x_byte_selected = !scroll_register_.is_x_byte_selected;
            return true;
        }
        case 0x06: {
            if (address_register_.is_high_byte_selected) {
                address_register_.HI = data;
            }
            else {
                address_register_.LO = data;
            }
            address_register_.is_high_byte_selected = !address_register_.is_high_byte_selected;
            return true;
        }
        case 0x07:
            data_buffer_ = data;
            return bus_->writeBusData(address_register_.address, data);
        default:
            return false;
    }
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

const std::array<RP2C02::Colour, RP2C02_PATTERN_TABLE_PIXEL_COUNT>& RP2C02::getPatternTableCache(const uint8_t& pattern_table_index, const uint8_t& palette_id) {
    const unsigned int chr_rom_pattern_table_byte_size = 0x1000;
    const unsigned int total_tile_rows = 16;
    const unsigned int total_tile_columns = 16;
    // Each Pattern Table is made from 16x16 tiles
    const unsigned int tiles_per_row = 16;
    const unsigned int tiles_per_column = 16;
    // Each tile is made from 8x8 pixels
    const unsigned int tile_pixel_per_side = 8;
    const unsigned int total_pixel_per_tile = tile_pixel_per_side * tile_pixel_per_side;
    // Each pixel is made from 2 bits, stored in 2 1-bit planes
    const unsigned int bit_per_pixel = 2;
    const unsigned int total_bytes_per_tile = total_pixel_per_tile * bit_per_pixel / 8;

    for (uint8_t tile_y = 0; tile_y < total_tile_rows; tile_y++) {
        for (uint8_t tile_x = 0; tile_x < total_tile_columns; tile_x++) {
            uint16_t flattened_tile_byte_index = tile_y * (total_tile_rows * total_bytes_per_tile) + tile_x * total_bytes_per_tile;
            // Read pixel data from a tile
            for (uint8_t pixel_y = 0; pixel_y < tile_pixel_per_side; pixel_y++) {
                uint8_t cur_row_pixel_lsb = bus_->readBusData(pattern_table_index * chr_rom_pattern_table_byte_size + flattened_tile_byte_index + pixel_y + 0);
                uint8_t cur_row_pixel_msb = bus_->readBusData(pattern_table_index * chr_rom_pattern_table_byte_size + flattened_tile_byte_index + pixel_y + 8);

                for (int8_t pixel_x = tile_pixel_per_side - 1; pixel_x >= 0; pixel_x--) {
                    uint8_t pixel_colour_value = (cur_row_pixel_msb & 0x01) << 1 | (cur_row_pixel_lsb & 0x01);
                    cur_row_pixel_msb >>= 1;
                    cur_row_pixel_lsb >>= 1;

                    // Update pattern_table_cache_
                    const uint16_t total_pixel_per_name_table_row = tiles_per_row * tile_pixel_per_side;
                    const uint16_t absolute_pixel_y = tile_y * tile_pixel_per_side + pixel_y;
                    const uint16_t absolute_pixel_x = tile_x * tile_pixel_per_side + pixel_x;
                    uint16_t flattened_pixel_index = (absolute_pixel_y * total_pixel_per_name_table_row) + absolute_pixel_x;
                    pattern_table_cache_[pattern_table_index].at(flattened_pixel_index) = getColourFromPalette(palette_id, pixel_colour_value);

                    // Colour result_colour = {0, 0, 0};
                    // switch (pixel_colour_value) {
                    //     case 0:
                    //         result_colour = {0, 0, 0};
                    //         break;
                    //     case 1:
                    //         result_colour = {255, 255, 255};
                    //         break;
                    //     case 2:
                    //         result_colour = {255, 0, 0};
                    //         break;
                    //     case 3:
                    //         result_colour = {0, 255, 0};
                    //         break;
                    // }
                    // pattern_table_cache_[pattern_table_index].at(flattened_pixel_index) = result_colour;
                }
            }
        }
    }

    return pattern_table_cache_[pattern_table_index];
}

RP2C02::Colour RP2C02::getColourFromPalette(const uint8_t& palette_id, const uint8_t& pixel_colour_value) const {
    return colour_palette_.at(bus_->readBusData(0x3F00 + (palette_id * 4) + pixel_colour_value));
}

void RP2C02::getTile(const uint8_t& tile_index, const uint8_t& pattern_table_index, const uint8_t& palette_id, Tile& out) const {
    const unsigned int total_bytes_per_tile = 16; // 8x8 pixels, 2 bits per pixel
    const uint16_t flattened_tile_byte_index = tile_index * total_bytes_per_tile;

    // Read pixel data from a tile
    const unsigned int tile_pixel_per_side = 8;
    const unsigned int chr_rom_pattern_table_byte_size = 0x1000;
    for (uint8_t pixel_y = 0; pixel_y < tile_pixel_per_side; pixel_y++) {
        uint8_t cur_row_pixel_lsb = bus_->readBusData(pattern_table_index * chr_rom_pattern_table_byte_size + flattened_tile_byte_index + pixel_y + 0);
        uint8_t cur_row_pixel_msb = bus_->readBusData(pattern_table_index * chr_rom_pattern_table_byte_size + flattened_tile_byte_index + pixel_y + 8);

        for (int8_t pixel_x = tile_pixel_per_side - 1; pixel_x >= 0; pixel_x--) {
            uint8_t pixel_colour_value = (cur_row_pixel_msb & 0x01) << 1 | (cur_row_pixel_lsb & 0x01);
            cur_row_pixel_msb >>= 1;
            cur_row_pixel_lsb >>= 1;

            Colour result_colour = getColourFromPalette(palette_id, pixel_colour_value);
            switch (pixel_colour_value) {
                case 0:
                    result_colour = {0, 0, 0};
                    break;
                case 1:
                    result_colour = {255, 255, 255};
                    break;
                case 2:
                    result_colour = {255, 0, 0};
                    break;
                case 3:
                    result_colour = {0, 255, 0};
                    break;
            }
            out.pixel_colour.at(pixel_y * tile_pixel_per_side + pixel_x) = result_colour;
        }
    }
}

uint16_t RP2C02::mirrorVRAMAddress(const uint16_t& address) const {
    // Mirroring the address
    if (address >= 0x2000 && address <= 0x3EFF) {
        return address - 0x2000;
    }
    return address;
}
