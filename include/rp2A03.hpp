#ifndef _RP2A03_HPP_
#define _RP2A03_HPP_
// Project Headers
#include "mos6502.hpp"

class RP2A03 : public MOS6502 {
public:
    // Constructor
    RP2A03();

    /**
     * @brief  Runs 1 cycle of the CPU
     * @param  None
     * @return None
    */
    void runCycle();

    /**
     * @brief  Starts a Direct Memory Access transfer
     * @param  page: The page to transfer from
     * @return None
    */
    void startDMATransfer(const uint8_t& page);

private:
    // Keeping track of the CPU clock ticks
    uint64_t clock_count_;

    // Helper Variables for DMA
    uint8_t dma_page_;
    uint8_t dma_address_;
    uint8_t dma_data_;
    bool dma_transfer_in_progress_;
    bool dma_is_synced_;
};

#endif
