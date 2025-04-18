#ifndef _RP2A03_HPP_
#define _RP2A03_HPP_
// Project Headers
#include "mos6502.hpp"

class RP2A03 {
public:
    // Constructor
    RP2A03();

    // Public methods
    /**
     * @brief  Connects the CPU to the BUS
     * @param  None
     * @return None
    */
    void connectBUS(BUS* target_bus);

    /**
     * @brief  Resets the CPU
     * @param  None
     * @return None
    */
    void reset();

    /**
     * @brief  Runs 1 cycle of the CPU
     * @param  None
     * @return None
    */
    void runCycle();

    /**
     * @brief  Non-maskable Interrupt Request
     * @param  None
     * @return None
    */
    void nmi();

    /**
     * @brief  Gets the current state of the CPU
     * @param  None
     * @return The current state of the CPU
    */
    MOS6502::State getState() const;

    /**
     * @brief  Disassembles the instruction at the given address
     * @param  address: The address to decode the instruction from
     * @return The decoded instruction and the length of the instruction
    */
    std::pair<std::string, uint8_t> disassembleInstruction(const uint16_t& address) const;

    /**
     * @brief  Starts a Direct Memory Access transfer
     * @param  page: The page to transfer from
     * @return None
    */
    void startDMATransfer(const uint8_t& page);

private:
    // Keeping track of the CPU clock ticks
    uint64_t clock_count_;

    // Internal hardwares
    MOS6502 mos6502_;

    // External hardwares
    BUS* bus_;

    // Helper Variables for DMA
    uint8_t dma_page_;
    uint8_t dma_address_;
    uint8_t dma_data_;
    bool dma_transfer_in_progress_;
    bool dma_is_synced_;
};

#endif
