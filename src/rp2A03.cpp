#include "rp2A03.hpp"

RP2A03::RP2A03(): 
    clock_count_(0),
    mos6502_(),
    bus_(nullptr),
    dma_page_(0),
    dma_address_(0),
    dma_data_(0),
    dma_transfer_in_progress_(false),
    dma_is_synced_(false) {}

void RP2A03::connectBUS(BUS* target_bus) {
    bus_ = target_bus;
    mos6502_.connectBUS(target_bus);
}

void RP2A03::reset() {
    mos6502_.reset();
}

void RP2A03::runCycle() {
    clock_count_++;
    
    if (dma_transfer_in_progress_) {
        if (!dma_is_synced_) {
            // Sync DMA transfer at odd CPU cycle
            dma_is_synced_ = clock_count_ % 2;
        }
        else {
            if (clock_count_ % 2 == 0) {
                dma_data_ = bus_->readBusData((dma_page_ << 8) | dma_address_);
            }
            else {
                bus_->writeBusData(0x2004, dma_data_);

                // Check if the DMA transfer is done
                if (dma_address_ == 0xFF) {
                    dma_transfer_in_progress_ = false;
                    dma_is_synced_ = false;
                }
                else {
                    dma_address_++;
                }
            }
        }
        return;
    }
    
    mos6502_.runCycle();
}

void RP2A03::nmi() {
    mos6502_.nmi();
}

MOS6502::State RP2A03::getState() const {
    return mos6502_.getState();
}

std::pair<std::string, uint8_t> RP2A03::disassembleInstruction(const uint16_t& address) const {
    return mos6502_.disassembleInstruction(address);
}

void RP2A03::startDMATransfer(const uint8_t& page) {
    dma_page_ = page;
    dma_address_ = 0x00;
    dma_transfer_in_progress_ = true;
    dma_is_synced_ = false;
}
