#include "memory-unit.hpp"

MemoryUnit::MemoryUnit(const uint32_t& byte_size): byte_size_(byte_size), memory_block_(std::make_unique<uint8_t[]>(byte_size)) {
    for (uint32_t i = 0; i < byte_size_; i++) {
        memory_block_[i] = 0x00;
    }
}

uint8_t MemoryUnit::read(const uint16_t& address) const {
    return memory_block_[address];
}

bool MemoryUnit::write(const uint16_t& address, const uint8_t& data) const {
    if (address > byte_size_) {
        return false;
    }
    memory_block_[address] = data;
    return true;
}

uint8_t* MemoryUnit::getPointer() const {
    return memory_block_.get();
}

uint32_t MemoryUnit::getSize() const {
    return byte_size_;
}
