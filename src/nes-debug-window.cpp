#include "nes-debug-window.hpp"

template< typename T >
static std::string int_to_hex(T i) {
  std::stringstream stream;
  stream << "0x" 
         << std::uppercase
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << static_cast<int64_t>(i);
  return stream.str();
}

NESDebugWindow::NESDebugWindow(): 
    nes_{nullptr}, window_{sf::VideoMode({NES_DEBUG_WINDOW_WIDTH, NES_DEBUG_WINDOW_HEIGHT}), NES_DEBUG_WINDOW_TITLE},
    name_table_0_texture_{sf::Vector2u{NES_DEBUG_WINDOW_NAME_TABLE_WIDTH, NES_DEBUG_WINDOW_NAME_TABLE_HEIGHT}},
    name_table_1_texture_{sf::Vector2u{NES_DEBUG_WINDOW_NAME_TABLE_WIDTH, NES_DEBUG_WINDOW_NAME_TABLE_HEIGHT}},
    name_table_0_sprite_{name_table_0_texture_},
    name_table_1_sprite_{name_table_1_texture_},
    name_table_0_pixel_buffer_{std::make_unique<uint8_t[]>(NES_DEBUG_WINDOW_NAME_TABLE_WIDTH * NES_DEBUG_WINDOW_NAME_TABLE_HEIGHT * 4)},
    name_table_1_pixel_buffer_{std::make_unique<uint8_t[]>(NES_DEBUG_WINDOW_NAME_TABLE_WIDTH * NES_DEBUG_WINDOW_NAME_TABLE_HEIGHT * 4)} {
    window_.setFramerateLimit(60);
    name_table_0_sprite_.setPosition({60, 520});
    name_table_1_sprite_.setPosition({128 + NES_DEBUG_WINDOW_NAME_TABLE_WIDTH, 520});
}

NESDebugWindow::~NESDebugWindow() {
    window_.close();
}

void NESDebugWindow::attachNES(NES* nes) {
    nes_ = nes;
}

void NESDebugWindow::update() {
    MOS6502::State cpu_state = nes_->cpu_.getState();
    

    window_.clear(sf::Color::Blue);
    sf::Font font;
    if(!font.openFromFile("./debug/JetBrainsMono.ttf")) {
        return;
    }

    sf::Text text(font);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::White);

    text.setString("CPU State:\n");
    text.setPosition({10, 10});
    window_.draw(text);
    
    text.setString(" A: " + int_to_hex(cpu_state.accumulator) + "\n");
    text.setPosition({10, 25});
    window_.draw(text);

    text.setString(" X: " + int_to_hex(cpu_state.x_reg) + "\n");
    text.setPosition({10, 40});
    window_.draw(text);

    text.setString(" Y: " + int_to_hex(cpu_state.y_reg) + "\n");
    text.setPosition({10, 55});
    window_.draw(text);

    text.setString("PC: " + int_to_hex(cpu_state.program_counter) + "\n");
    text.setPosition({10, 70});
    window_.draw(text);

    text.setString("SP: " + int_to_hex(cpu_state.stack_ptr) + "\n");
    text.setPosition({10, 85});
    window_.draw(text);

    text.setString(" P: " + int_to_hex(cpu_state.processor_status) + "\n");
    text.setPosition({10, 100});
    window_.draw(text);
    
    uint16_t address_to_disassemble = cpu_state.program_counter;
    for (int i = 0; i <= 5; i++) {
        auto[disassembled_instruction_text, instruction_size] = nes_->cpu_.disassembleInstruction(address_to_disassemble);
        text.setString(disassembled_instruction_text + "\n");
        text.setPosition({10, static_cast<float>(130 + (i * 15))});
        window_.draw(text);
        address_to_disassemble += instruction_size;
    }

    RP2C02::NameTable* name_table = reinterpret_cast<RP2C02::NameTable*>(nes_->vram_.getPointer());
    for (uint8_t name_table_index = 0; name_table_index < 2; name_table_index++) {
        for (uint8_t tile_y = 0; tile_y < 30; tile_y++) {
            for (uint8_t tile_x = 0; tile_x < 32; tile_x++) {
                uint16_t flattened_name_table_tile_index = tile_y * 32 + tile_x;

                uint8_t attribute_block_index = (tile_y / 4) * 8 + (tile_x / 4);
                uint8_t palette_id = name_table->palette_data.at(attribute_block_index);

                uint8_t attribute_block_x = tile_x % 4;
                uint8_t attribute_block_y = tile_y % 4;
                if ((attribute_block_x <= 1) && (attribute_block_y <= 1)) {
                    // Quad 0 of the attribute block
                    palette_id = (palette_id & 0b00000011);
                } else if ((attribute_block_x > 1) && (attribute_block_y <= 1)) {
                    // Quad 1 of the attribute block
                    palette_id = (palette_id & 0b00001100) >> 2;
                } else if ((attribute_block_x <= 1) && (attribute_block_y > 1)) {
                    // Quad 2 of the attribute block
                    palette_id = (palette_id & 0b00110000) >> 4;
                } else {
                    // Quad 3 of the attribute block
                    palette_id = (palette_id & 0b11000000) >> 6;
                }

                uint8_t flattened_pattern_table_tile_index = name_table->tile_data.at(flattened_name_table_tile_index);
                RP2C02::Tile tile = nes_->ppu_.getTileFromPatternTable(flattened_pattern_table_tile_index, name_table_index, palette_id);
                
                for (uint8_t pixel_y = 0; pixel_y < 8; pixel_y++) {
                    for (uint8_t pixel_x = 0; pixel_x < 8; pixel_x++) {
                        NESWindow::Colour& cur_pixel_colour = tile.pixel_colour.at(pixel_y * 8 + pixel_x);

                        if (name_table_index == 0) {
                            name_table_0_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 0] = cur_pixel_colour.r;
                            name_table_0_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 1] = cur_pixel_colour.g;
                            name_table_0_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 2] = cur_pixel_colour.b;
                            name_table_0_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 3] = 255; // Solid Alpha
                        } else {
                            name_table_1_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 0] = cur_pixel_colour.r;
                            name_table_1_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 1] = cur_pixel_colour.g;
                            name_table_1_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 2] = cur_pixel_colour.b;
                            name_table_1_pixel_buffer_[((tile_y * 8 + pixel_y) * NES_DEBUG_WINDOW_NAME_TABLE_WIDTH + (tile_x * 8 + pixel_x)) * 4 + 3] = 255; // Solid Alpha
                        }
                    }
                }
            }
        }
    }

    name_table_0_texture_.update(name_table_0_pixel_buffer_.get());
    name_table_1_texture_.update(name_table_1_pixel_buffer_.get());
    window_.draw(name_table_0_sprite_);
    window_.draw(name_table_1_sprite_);
    window_.display();
}
