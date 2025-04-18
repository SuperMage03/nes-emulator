#include "rp2A03.hpp"

RP2A03::RP2A03(): 
    clock_count_(0),
    dma_page_(0),
    dma_address_(0),
    dma_data_(0),
    dma_transfer_in_progress_(false),
    dma_is_synced_(false) {}

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
    
    MOS6502::runCycle();
}

void RP2A03::startDMATransfer(const uint8_t& page) {
    dma_page_ = page;
    dma_address_ = 0x00;
    dma_transfer_in_progress_ = true;
    dma_is_synced_ = false;
}
