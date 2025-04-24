#include "mos6502.hpp"
// Stardard Library Headers
#include <bitset>
#include <iomanip>
#include <sstream>
// Project Headers
#include "bus.hpp"

// ------------------------ MOS6502 Pointer Class ------------------------------

MOS6502::Pointer::Pointer(MOS6502& cpu, const uint16_t& virtual_address): 
    cpu_{cpu}, target_location_{std::in_place_type<uint16_t>, virtual_address} {}

MOS6502::Pointer::Pointer(MOS6502& cpu, const MOS6502::Pointer::Register& target_register): 
    cpu_{cpu}, target_location_{std::in_place_type<MOS6502::Pointer::Register>, target_register} {}

const std::variant<uint16_t, MOS6502::Pointer::Register>& MOS6502::Pointer::get() const {
    return target_location_;
}

uint8_t MOS6502::Pointer::read() const {
    // Holds Virtual Memory Address
    if (std::holds_alternative<uint16_t>(target_location_)) {
        return cpu_.readMemory(std::get<uint16_t>(target_location_));
    }
    // Holds "Address" for Register
    switch (std::get<MOS6502::Pointer::Register>(target_location_)) {
        case MOS6502::Pointer::Register::ACCUMULATOR:
            return cpu_.accumulator_;
    }
}

void MOS6502::Pointer::write(const uint8_t& data) {
    // Holds Virtual Memory Address
    if (std::holds_alternative<uint16_t>(target_location_)) {
        cpu_.writeMemory(std::get<uint16_t>(target_location_), data);
        return;
    }
    // Holds "Address" for Register
    switch (std::get<MOS6502::Pointer::Register>(target_location_)) {
        case MOS6502::Pointer::Register::ACCUMULATOR:
            cpu_.accumulator_ = data;
    }
}

void MOS6502::Pointer::operator=(const uint16_t& virtual_address) {
    target_location_ = virtual_address;
}

void MOS6502::Pointer::operator=(const MOS6502::Pointer::Register& target_register) {
    target_location_ = target_register;
}

MOS6502::Pointer& MOS6502::Pointer::operator++() {
    return *this += 1;
}

MOS6502::Pointer& MOS6502::Pointer::operator+=(const int16_t& increment) {
    // Holds Virtual Memory Address
    if (std::holds_alternative<uint16_t>(target_location_)) {
        std::get<uint16_t>(target_location_) += increment;
    }
    return *this;
}

// ----------------------------- MOS6502 Class ---------------------------------

// Thanks to One Lone Coder for the opcode table
const std::array<MOS6502::Instruction, MOS6502_NUMBER_OF_INSTRUCTIONS> MOS6502::instruction_lookup_table = {{
    { "BRK", MOS6502::BRK, MOS6502::IMM, 7 },{ "ORA", MOS6502::ORA, MOS6502::IZX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 3 },{ "ORA", MOS6502::ORA, MOS6502::ZP0, 3 },{ "ASL", MOS6502::ASL, MOS6502::ZP0, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "PHP", MOS6502::PHP, MOS6502::IMP, 3 },{ "ORA", MOS6502::ORA, MOS6502::IMM, 2 },{ "ASL", MOS6502::ASL, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "ORA", MOS6502::ORA, MOS6502::ABS, 4 },{ "ASL", MOS6502::ASL, MOS6502::ABS, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },
    { "BPL", MOS6502::BPL, MOS6502::REL, 2 },{ "ORA", MOS6502::ORA, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "ORA", MOS6502::ORA, MOS6502::ZPX, 4 },{ "ASL", MOS6502::ASL, MOS6502::ZPX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "CLC", MOS6502::CLC, MOS6502::IMP, 2 },{ "ORA", MOS6502::ORA, MOS6502::ABY, 4 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "ORA", MOS6502::ORA, MOS6502::ABX, 4 },{ "ASL", MOS6502::ASL, MOS6502::ABX, 7 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },
    { "JSR", MOS6502::JSR, MOS6502::ABS, 6 },{ "AND", MOS6502::AND, MOS6502::IZX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "BIT", MOS6502::BIT, MOS6502::ZP0, 3 },{ "AND", MOS6502::AND, MOS6502::ZP0, 3 },{ "ROL", MOS6502::ROL, MOS6502::ZP0, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "PLP", MOS6502::PLP, MOS6502::IMP, 4 },{ "AND", MOS6502::AND, MOS6502::IMM, 2 },{ "ROL", MOS6502::ROL, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "BIT", MOS6502::BIT, MOS6502::ABS, 4 },{ "AND", MOS6502::AND, MOS6502::ABS, 4 },{ "ROL", MOS6502::ROL, MOS6502::ABS, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },
    { "BMI", MOS6502::BMI, MOS6502::REL, 2 },{ "AND", MOS6502::AND, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "AND", MOS6502::AND, MOS6502::ZPX, 4 },{ "ROL", MOS6502::ROL, MOS6502::ZPX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "SEC", MOS6502::SEC, MOS6502::IMP, 2 },{ "AND", MOS6502::AND, MOS6502::ABY, 4 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "AND", MOS6502::AND, MOS6502::ABX, 4 },{ "ROL", MOS6502::ROL, MOS6502::ABX, 7 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },
    { "RTI", MOS6502::RTI, MOS6502::IMP, 6 },{ "EOR", MOS6502::EOR, MOS6502::IZX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 3 },{ "EOR", MOS6502::EOR, MOS6502::ZP0, 3 },{ "LSR", MOS6502::LSR, MOS6502::ZP0, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "PHA", MOS6502::PHA, MOS6502::IMP, 3 },{ "EOR", MOS6502::EOR, MOS6502::IMM, 2 },{ "LSR", MOS6502::LSR, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "JMP", MOS6502::JMP, MOS6502::ABS, 3 },{ "EOR", MOS6502::EOR, MOS6502::ABS, 4 },{ "LSR", MOS6502::LSR, MOS6502::ABS, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },
    { "BVC", MOS6502::BVC, MOS6502::REL, 2 },{ "EOR", MOS6502::EOR, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "EOR", MOS6502::EOR, MOS6502::ZPX, 4 },{ "LSR", MOS6502::LSR, MOS6502::ZPX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "CLI", MOS6502::CLI, MOS6502::IMP, 2 },{ "EOR", MOS6502::EOR, MOS6502::ABY, 4 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "EOR", MOS6502::EOR, MOS6502::ABX, 4 },{ "LSR", MOS6502::LSR, MOS6502::ABX, 7 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },
    { "RTS", MOS6502::RTS, MOS6502::IMP, 6 },{ "ADC", MOS6502::ADC, MOS6502::IZX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 3 },{ "ADC", MOS6502::ADC, MOS6502::ZP0, 3 },{ "ROR", MOS6502::ROR, MOS6502::ZP0, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "PLA", MOS6502::PLA, MOS6502::IMP, 4 },{ "ADC", MOS6502::ADC, MOS6502::IMM, 2 },{ "ROR", MOS6502::ROR, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "JMP", MOS6502::JMP, MOS6502::IND, 5 },{ "ADC", MOS6502::ADC, MOS6502::ABS, 4 },{ "ROR", MOS6502::ROR, MOS6502::ABS, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },
    { "BVS", MOS6502::BVS, MOS6502::REL, 2 },{ "ADC", MOS6502::ADC, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "ADC", MOS6502::ADC, MOS6502::ZPX, 4 },{ "ROR", MOS6502::ROR, MOS6502::ZPX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "SEI", MOS6502::SEI, MOS6502::IMP, 2 },{ "ADC", MOS6502::ADC, MOS6502::ABY, 4 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "ADC", MOS6502::ADC, MOS6502::ABX, 4 },{ "ROR", MOS6502::ROR, MOS6502::ABX, 7 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },
    { "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "STA", MOS6502::STA, MOS6502::IZX, 6 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "STY", MOS6502::STY, MOS6502::ZP0, 3 },{ "STA", MOS6502::STA, MOS6502::ZP0, 3 },{ "STX", MOS6502::STX, MOS6502::ZP0, 3 },{ "???", MOS6502::XXX, MOS6502::IMP, 3 },{ "DEY", MOS6502::DEY, MOS6502::IMP, 2 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "TXA", MOS6502::TXA, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "STY", MOS6502::STY, MOS6502::ABS, 4 },{ "STA", MOS6502::STA, MOS6502::ABS, 4 },{ "STX", MOS6502::STX, MOS6502::ABS, 4 },{ "???", MOS6502::XXX, MOS6502::IMP, 4 },
    { "BCC", MOS6502::BCC, MOS6502::REL, 2 },{ "STA", MOS6502::STA, MOS6502::IZY, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "STY", MOS6502::STY, MOS6502::ZPX, 4 },{ "STA", MOS6502::STA, MOS6502::ZPX, 4 },{ "STX", MOS6502::STX, MOS6502::ZPY, 4 },{ "???", MOS6502::XXX, MOS6502::IMP, 4 },{ "TYA", MOS6502::TYA, MOS6502::IMP, 2 },{ "STA", MOS6502::STA, MOS6502::ABY, 5 },{ "TXS", MOS6502::TXS, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "???", MOS6502::NOP, MOS6502::IMP, 5 },{ "STA", MOS6502::STA, MOS6502::ABX, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },
    { "LDY", MOS6502::LDY, MOS6502::IMM, 2 },{ "LDA", MOS6502::LDA, MOS6502::IZX, 6 },{ "LDX", MOS6502::LDX, MOS6502::IMM, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "LDY", MOS6502::LDY, MOS6502::ZP0, 3 },{ "LDA", MOS6502::LDA, MOS6502::ZP0, 3 },{ "LDX", MOS6502::LDX, MOS6502::ZP0, 3 },{ "???", MOS6502::XXX, MOS6502::IMP, 3 },{ "TAY", MOS6502::TAY, MOS6502::IMP, 2 },{ "LDA", MOS6502::LDA, MOS6502::IMM, 2 },{ "TAX", MOS6502::TAX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "LDY", MOS6502::LDY, MOS6502::ABS, 4 },{ "LDA", MOS6502::LDA, MOS6502::ABS, 4 },{ "LDX", MOS6502::LDX, MOS6502::ABS, 4 },{ "???", MOS6502::XXX, MOS6502::IMP, 4 },
    { "BCS", MOS6502::BCS, MOS6502::REL, 2 },{ "LDA", MOS6502::LDA, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "LDY", MOS6502::LDY, MOS6502::ZPX, 4 },{ "LDA", MOS6502::LDA, MOS6502::ZPX, 4 },{ "LDX", MOS6502::LDX, MOS6502::ZPY, 4 },{ "???", MOS6502::XXX, MOS6502::IMP, 4 },{ "CLV", MOS6502::CLV, MOS6502::IMP, 2 },{ "LDA", MOS6502::LDA, MOS6502::ABY, 4 },{ "TSX", MOS6502::TSX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 4 },{ "LDY", MOS6502::LDY, MOS6502::ABX, 4 },{ "LDA", MOS6502::LDA, MOS6502::ABX, 4 },{ "LDX", MOS6502::LDX, MOS6502::ABY, 4 },{ "???", MOS6502::XXX, MOS6502::IMP, 4 },
    { "CPY", MOS6502::CPY, MOS6502::IMM, 2 },{ "CMP", MOS6502::CMP, MOS6502::IZX, 6 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "CPY", MOS6502::CPY, MOS6502::ZP0, 3 },{ "CMP", MOS6502::CMP, MOS6502::ZP0, 3 },{ "DEC", MOS6502::DEC, MOS6502::ZP0, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "INY", MOS6502::INY, MOS6502::IMP, 2 },{ "CMP", MOS6502::CMP, MOS6502::IMM, 2 },{ "DEX", MOS6502::DEX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "CPY", MOS6502::CPY, MOS6502::ABS, 4 },{ "CMP", MOS6502::CMP, MOS6502::ABS, 4 },{ "DEC", MOS6502::DEC, MOS6502::ABS, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },
    { "BNE", MOS6502::BNE, MOS6502::REL, 2 },{ "CMP", MOS6502::CMP, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "CMP", MOS6502::CMP, MOS6502::ZPX, 4 },{ "DEC", MOS6502::DEC, MOS6502::ZPX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "CLD", MOS6502::CLD, MOS6502::IMP, 2 },{ "CMP", MOS6502::CMP, MOS6502::ABY, 4 },{ "NOP", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "CMP", MOS6502::CMP, MOS6502::ABX, 4 },{ "DEC", MOS6502::DEC, MOS6502::ABX, 7 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },
    { "CPX", MOS6502::CPX, MOS6502::IMM, 2 },{ "SBC", MOS6502::SBC, MOS6502::IZX, 6 },{ "???", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "CPX", MOS6502::CPX, MOS6502::ZP0, 3 },{ "SBC", MOS6502::SBC, MOS6502::ZP0, 3 },{ "INC", MOS6502::INC, MOS6502::ZP0, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 5 },{ "INX", MOS6502::INX, MOS6502::IMP, 2 },{ "SBC", MOS6502::SBC, MOS6502::IMM, 2 },{ "NOP", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::SBC, MOS6502::IMP, 2 },{ "CPX", MOS6502::CPX, MOS6502::ABS, 4 },{ "SBC", MOS6502::SBC, MOS6502::ABS, 4 },{ "INC", MOS6502::INC, MOS6502::ABS, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },
    { "BEQ", MOS6502::BEQ, MOS6502::REL, 2 },{ "SBC", MOS6502::SBC, MOS6502::IZY, 5 },{ "???", MOS6502::XXX, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 8 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "SBC", MOS6502::SBC, MOS6502::ZPX, 4 },{ "INC", MOS6502::INC, MOS6502::ZPX, 6 },{ "???", MOS6502::XXX, MOS6502::IMP, 6 },{ "SED", MOS6502::SED, MOS6502::IMP, 2 },{ "SBC", MOS6502::SBC, MOS6502::ABY, 4 },{ "NOP", MOS6502::NOP, MOS6502::IMP, 2 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },{ "???", MOS6502::NOP, MOS6502::IMP, 4 },{ "SBC", MOS6502::SBC, MOS6502::ABX, 4 },{ "INC", MOS6502::INC, MOS6502::ABX, 7 },{ "???", MOS6502::XXX, MOS6502::IMP, 7 },
}};

MOS6502::MOS6502(): bus_(nullptr), program_counter_(MOS6502_STARTING_PC_ADDRESS), stack_ptr_(0), accumulator_(0), 
                    x_reg_(0), y_reg_(0), processor_status_({.RAW_VALUE=0b00110110}),
                    cycles_elapsed_(0), instruction_(nullptr), instruction_opcode_(0x00), 
                    instruction_cycle_remaining_(0), operand_address_(*this, 0x00), 
                    relative_addressing_offset_(0) {}

void MOS6502::connectBUS(BUS* target_bus) {
    bus_ = target_bus;
    // Do a reset to clear CPU states
    reset();
}

void MOS6502::runInstruction() {
    instruction_opcode_ = readMemory(program_counter_);
    program_counter_++;

    instruction_ = &instruction_lookup_table.at(instruction_opcode_);
    instruction_cycle_remaining_ = instruction_->cycles;

    // Getting the additional cycles from the addressing mode
    uint8_t additional_cycles = instruction_->addressingMode(*this);
    // Note calling instruction_->operationFn(*this) can change instruction_cycle_remaining_
    //   This is only done by branching instructions since their additional cycles are independent of the addressing mode
    CycleType instruction_cycle_mode = instruction_->operationFn(*this);

    if (instruction_cycle_mode == CycleType::ACCEPTS_ADDITIONAL_CYCLES) {
        instruction_cycle_remaining_ += additional_cycles;
    }
    
    cycles_elapsed_ += instruction_cycle_remaining_;
}

void MOS6502::runCycle() {
    cycles_elapsed_++;

    // Fetch a new instruction when the current instruction is done
    if (instruction_cycle_remaining_ == 0) {
        instruction_opcode_ = readMemory(program_counter_);
        program_counter_++;

        instruction_ = &instruction_lookup_table.at(instruction_opcode_);
        instruction_cycle_remaining_ = instruction_->cycles;

        // Getting the additional cycles from the addressing mode
        uint8_t additional_cycles = instruction_->addressingMode(*this);
        // Note calling instruction_->operationFn(*this) can change instruction_cycle_remaining_
        //   This is only done by branching instructions since their additional cycles are independent of the addressing mode
        CycleType instruction_cycle_mode = instruction_->operationFn(*this);

        if (instruction_cycle_mode == CycleType::ACCEPTS_ADDITIONAL_CYCLES) {
            instruction_cycle_remaining_ += additional_cycles;
        }
    }

    instruction_cycle_remaining_--;
}

std::pair<std::string, uint8_t> MOS6502::disassembleInstruction(const uint16_t& address) const {
    std::stringstream ss;
    uint8_t opcode = readMemory(address);
    const Instruction& instruction = instruction_lookup_table.at(opcode);

    ss << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(address);
    ss << ": ";
    ss << instruction.name;
    ss << " ";
    if (instruction.addressingMode == MOS6502::IMP) {
        return {ss.str(), 1};
    }
    if (instruction.addressingMode == MOS6502::IMM) {
        ss << "#$" << std::hex << static_cast<int>(readMemory(address + 1));
        return {ss.str(), 2};
    }
    if (instruction.addressingMode == MOS6502::ZP0) {
        ss << "$" << std::hex << static_cast<int>(readMemory(address + 1));
        return {ss.str(), 2};
    }
    if (instruction.addressingMode == MOS6502::ZPX) {
        ss << "$" << std::hex << static_cast<int>(readMemory(address + 1)) << ",X";
        return {ss.str(), 2};
    }
    if (instruction.addressingMode == MOS6502::ZPY) {
        ss << "$" << std::hex << static_cast<int>(readMemory(address + 1)) << ",Y";
        return {ss.str(), 2};
    }
    if (instruction.addressingMode == MOS6502::IZX) {
        ss << "($" << std::hex << static_cast<int>(readMemory(address + 1)) << ",X)";
        return {ss.str(), 2};
    }
    if (instruction.addressingMode == MOS6502::IZY) {
        ss << "($" << std::hex << static_cast<int>(readMemory(address + 1)) << "),Y";
        return {ss.str(), 2};
    }
    if (instruction.addressingMode == MOS6502::ABS) {
        ss << "$" << std::hex 
           << (static_cast<int>(readMemory(address + 2)) | (static_cast<int>(readMemory(address + 1)) << 8));
        return {ss.str(), 3};
    }
    if (instruction.addressingMode == MOS6502::ABX) {
        ss << "$" 
           << std::hex 
           << (static_cast<int>(readMemory(address + 2)) | (static_cast<int>(readMemory(address + 1)) << 8))
           << ",X";
        return {ss.str(), 3};
    }
    if (instruction.addressingMode == MOS6502::ABY) {
        ss << "$" 
           << std::hex 
           << (static_cast<int>(readMemory(address + 2)) | (static_cast<int>(readMemory(address + 1)) << 8))
           << ",Y";
        return {ss.str(), 3};
    }
    if (instruction.addressingMode == MOS6502::IND) {
        ss << "($" 
           << std::hex 
           << (static_cast<int>(readMemory(address + 2)) | (static_cast<int>(readMemory(address + 1)) << 8))
           << ")";
        return {ss.str(), 3};
    }
    if (instruction.addressingMode == MOS6502::REL) {
        ss << "$" 
           << std::hex
           << (static_cast<int>(readMemory(address + 1)) + address + 2);
        return {ss.str(), 2};
    }
    return {"ERROR", 1};
}

// ------------------------ EXTERNAL INTERRUPTS --------------------------------

void MOS6502::reset() {
    // MOS6502 Registers
    uint16_t starting_pc_low_byte = readMemory(MOS6502_STARTING_PC_ADDRESS);
    uint16_t starting_pc_high_byte = readMemory(MOS6502_STARTING_PC_ADDRESS + 1);
    program_counter_ = (starting_pc_high_byte << 8) | starting_pc_low_byte;

    accumulator_ = 0;
    x_reg_ = 0;
    y_reg_ = 0;
    stack_ptr_ = 0xFD;

    processor_status_.RAW_VALUE = 0b00110110;

    // Emulator Variables
    cycles_elapsed_ = 0;

    // Variables needed for fetch->decode->execute cycle
    instruction_ = nullptr;
    instruction_opcode_ = 0;
    instruction_cycle_remaining_ = 8; // Reset takes time

    // Variables that emulates the data carried on a data-path
    operand_address_ = 0;
    relative_addressing_offset_ = 0;
}

void MOS6502::irq() {
    if (getStatusFlag(StatusFlag::INTERRUPT_DISABLE)) return;

    uint8_t pc_high_byte = (program_counter_ & 0xFF00) >> 8;
    uint8_t pc_low_byte = program_counter_ & 0x00FF;
    stackPush(pc_high_byte);
    stackPush(pc_low_byte);

    ProcessorStatus status_to_push = processor_status_;
    status_to_push.BREAK = 0;
    status_to_push.UNUSED = 1;
    status_to_push.INTERRUPT_DISABLE = 1;
    stackPush(status_to_push.RAW_VALUE);

    setStatusFlag(StatusFlag::INTERRUPT_DISABLE, 1);

    uint16_t irq_pc_low_byte = readMemory(MOS6502_IRQ_PC_ADDRESS);
    uint16_t irq_pc_high_byte = readMemory(MOS6502_IRQ_PC_ADDRESS + 1);
    program_counter_ = (irq_pc_high_byte << 8) | irq_pc_low_byte;
}

void MOS6502::nmi() {
    uint8_t pc_high_byte = (program_counter_ & 0xFF00) >> 8;
    uint8_t pc_low_byte = program_counter_ & 0x00FF;
    stackPush(pc_high_byte);
    stackPush(pc_low_byte);

    ProcessorStatus status_to_push = processor_status_;
    status_to_push.BREAK = 0;
    status_to_push.UNUSED = 1;
    status_to_push.INTERRUPT_DISABLE = 1;
    stackPush(status_to_push.RAW_VALUE);

    setStatusFlag(StatusFlag::INTERRUPT_DISABLE, 1);

    uint16_t nmi_pc_low_byte = readMemory(MOS6502_NMI_PC_ADDRESS);
    uint16_t nmi_pc_high_byte = readMemory(MOS6502_NMI_PC_ADDRESS + 1);
    program_counter_ = (nmi_pc_high_byte << 8) | nmi_pc_low_byte;
}

// ------------------------ INTERNAL FUNCTIONS ---------------------------------

uint64_t MOS6502::getCyclesElapsed() const {
    return cycles_elapsed_;
}

MOS6502::State MOS6502::getState() const {
    return State{program_counter_, stack_ptr_, accumulator_, x_reg_, y_reg_, processor_status_.RAW_VALUE};
}

void MOS6502::setState(const MOS6502::State& new_state) {
    program_counter_ = new_state.program_counter;
    stack_ptr_ = new_state.stack_ptr;
    accumulator_ = new_state.accumulator;
    x_reg_ = new_state.x_reg;
    y_reg_ = new_state.y_reg;
    processor_status_.RAW_VALUE = new_state.processor_status;
}

void MOS6502::outputCurrentState(std::ostream &out) const {
    out << std::hex;
    out << "Program Counter: 0x" << program_counter_ << std::endl;
    out << "Stack Pointer  : 0x" << static_cast<uint16_t>(stack_ptr_) << std::endl;
    out << "Accumulator    : 0x" << static_cast<uint16_t>(accumulator_) << std::endl;
    out << "X Register     : 0x" << static_cast<uint16_t>(x_reg_) << std::endl;
    out << "Y Register     : 0x" << static_cast<uint16_t>(y_reg_) << std::endl;
    out << "Status Flags   : 0b" << std::bitset<8>(processor_status_.RAW_VALUE) << std::endl;
    out << std::dec;
    out << "Cycles Elapsed : " << cycles_elapsed_ << std::endl;
}

uint8_t MOS6502::readMemory(const uint16_t& address) const {
    return bus_->readBusData(address);
}

bool MOS6502::writeMemory(const uint16_t& address, const uint8_t& data) {
    return bus_->writeBusData(address, data);
}

uint8_t MOS6502::getStatusFlag(const StatusFlag& flag) const {
    uint8_t bit_mask = (1 << static_cast<uint8_t>(flag));
    return (processor_status_.RAW_VALUE & bit_mask) > 0;
}

void MOS6502::setStatusFlag(const StatusFlag& flag, const uint16_t& value) {
    uint8_t bit_mask = (1 << static_cast<uint8_t>(flag));
    if (value > 0) {
        processor_status_.RAW_VALUE |= bit_mask;
    }
    else {
        processor_status_.RAW_VALUE &= ~bit_mask;
    }
}

uint8_t MOS6502::stackPop() {
    stack_ptr_++;
    uint8_t top_item = readMemory(0x0100 + stack_ptr_);
    return top_item;
}

void MOS6502::stackPush(const uint8_t& data) {
    writeMemory(0x0100 + stack_ptr_, data);
    stack_ptr_--;
}

// ---------------------- INSTRUCTION IMPLEMENTATIONS --------------------------

MOS6502::CycleType MOS6502::ADC(MOS6502& cpu) {
    uint16_t result = static_cast<uint16_t>(cpu.accumulator_) + static_cast<uint16_t>(cpu.operand_address_.read()) + static_cast<uint16_t>(cpu.getStatusFlag(StatusFlag::CARRY));

	// The carry flag out exists in the high byte bit 0
	cpu.setStatusFlag(StatusFlag::CARRY, result > 0x00FF);
	
	// The Zero flag is set if the result is 0
	cpu.setStatusFlag(StatusFlag::ZERO, (result & 0x00FF) == 0);
	
	// The signed Overflow flag is set based on all that up there! :D
	cpu.setStatusFlag(StatusFlag::OVERFLOW_FLAG, (~(static_cast<uint16_t>(cpu.accumulator_) ^ static_cast<uint16_t>(cpu.operand_address_.read())) & (static_cast<uint16_t>(cpu.accumulator_) ^ static_cast<uint16_t>(result))) & 0x0080);
	
	// The negative flag is set to the most significant bit of the result
	cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x0080);
	
	// Load the result into the accumulator (it's 8-bit dont forget!)
	cpu.accumulator_ = result & 0x00FF;
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::AND(MOS6502& cpu) {
    cpu.accumulator_ &= cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0x00);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::ASL(MOS6502& cpu) {
    uint8_t result = cpu.operand_address_.read() << 0x01;

    cpu.setStatusFlag(StatusFlag::CARRY, cpu.operand_address_.read() & 0x80);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0x00);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x80);

    cpu.operand_address_.write(result);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BCC(MOS6502& cpu) {
    if (!cpu.getStatusFlag(StatusFlag::CARRY)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BCS(MOS6502& cpu) {
    if (cpu.getStatusFlag(StatusFlag::CARRY)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BEQ(MOS6502& cpu) {
    if (cpu.getStatusFlag(StatusFlag::ZERO)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BIT(MOS6502& cpu) {
    uint8_t result = cpu.operand_address_.read() & cpu.accumulator_;
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0x00);
    cpu.setStatusFlag(StatusFlag::OVERFLOW_FLAG, cpu.operand_address_.read() & 0x40);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.operand_address_.read() & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BMI(MOS6502& cpu) {
    if (cpu.getStatusFlag(StatusFlag::NEGATIVE)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BNE(MOS6502& cpu) {
    if (!cpu.getStatusFlag(StatusFlag::ZERO)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }

        if (cpu.program_counter_ == new_pc_address + 2) {
            // Stuck
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BPL(MOS6502& cpu) {
    if (!cpu.getStatusFlag(StatusFlag::NEGATIVE)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BRK(MOS6502& cpu) {
    uint8_t pc_low_byte = cpu.program_counter_ & 0x00FF;
    uint8_t pc_high_byte = (cpu.program_counter_ & 0xFF00) >> 8;

    cpu.stackPush(pc_high_byte);
    cpu.stackPush(pc_low_byte);

    // There is a padding byte for BRK instruction 
    //    (Just adding for the clarity, PC will be changed anyways at the end of the function)
    cpu.program_counter_++;

    // Break flag is only really "exist" when it's pushed to stack
    //   This is to distinguish between BRK and an IQR

    ProcessorStatus status_to_push = cpu.processor_status_;
    status_to_push.BREAK = 1;
    status_to_push.UNUSED = 1;
    cpu.stackPush(status_to_push.RAW_VALUE);

    // Make sure that CPU is not in Interrupt Mode After BRK
    cpu.setStatusFlag(StatusFlag::INTERRUPT_DISABLE, 1);

    uint16_t interrupt_vector_low_byte = cpu.readMemory(MOS6502_IRQ_PC_ADDRESS);
    uint16_t interrupt_vector_high_byte = cpu.readMemory(MOS6502_IRQ_PC_ADDRESS + 1);

    cpu.program_counter_ = (interrupt_vector_high_byte << 8) | interrupt_vector_low_byte;
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BVC(MOS6502& cpu) {
    if (!cpu.getStatusFlag(StatusFlag::OVERFLOW_FLAG)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::BVS(MOS6502& cpu) {
    if (cpu.getStatusFlag(StatusFlag::OVERFLOW_FLAG)) {
        uint16_t new_pc_address = cpu.program_counter_ + cpu.relative_addressing_offset_;
        // Branch success adds 1 cycle
        cpu.instruction_cycle_remaining_++;
        // If branched to a new page we need to add 1 more cycle
        if ((cpu.program_counter_ & 0xFF00) != (new_pc_address & 0xFF00)) {
            cpu.instruction_cycle_remaining_++;
        }
        cpu.program_counter_ = new_pc_address;
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CLC(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::CARRY, 0);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CLD(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::DECIMAL_MODE, 0);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CLI(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::INTERRUPT_DISABLE, 0);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CLV(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::OVERFLOW_FLAG, 0);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CMP(MOS6502& cpu) {
    int16_t result = cpu.accumulator_ - cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::CARRY, result >= 0);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x0080);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CPX(MOS6502& cpu) {
    int16_t result = cpu.x_reg_ - cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::CARRY, result >= 0);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x0080);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::CPY(MOS6502& cpu) {
    int16_t result = cpu.y_reg_ - cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::CARRY, result >= 0);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x0080);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::DEC(MOS6502& cpu) {
    cpu.operand_address_.write(cpu.operand_address_.read() - 1);
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.operand_address_.read() == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.operand_address_.read() & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::DEX(MOS6502& cpu) {
    cpu.x_reg_--;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.x_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.x_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::DEY(MOS6502& cpu) {
    cpu.y_reg_--;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.y_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.y_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::EOR(MOS6502& cpu) {
    cpu.accumulator_ ^= cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::INC(MOS6502& cpu) {
    cpu.operand_address_.write(cpu.operand_address_.read() + 1);
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.operand_address_.read() == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.operand_address_.read() & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::INX(MOS6502& cpu) {
    cpu.x_reg_++;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.x_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.x_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::INY(MOS6502& cpu) {
    cpu.y_reg_++;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.y_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.y_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::JMP(MOS6502& cpu) {
    const std::variant<uint16_t, MOS6502::Pointer::Register>& jump_location = cpu.operand_address_.get();
    if (std::holds_alternative<uint16_t>(jump_location)) {
        cpu.program_counter_ = std::get<uint16_t>(jump_location);
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::JSR(MOS6502& cpu) {
    const std::variant<uint16_t, MOS6502::Pointer::Register>& jump_location = cpu.operand_address_.get();
    if (std::holds_alternative<uint16_t>(jump_location)) {
        uint16_t return_address = cpu.program_counter_ - 1;
        uint8_t return_address_high_byte = (return_address & 0xFF00) >> 8;
        uint8_t return_address_low_byte = return_address & 0x00FF;
        cpu.stackPush(return_address_high_byte);
        cpu.stackPush(return_address_low_byte);
        cpu.program_counter_ = std::get<uint16_t>(jump_location);
    }
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::LDA(MOS6502& cpu) {
    cpu.accumulator_ = cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::LDX(MOS6502& cpu) {
    cpu.x_reg_ = cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.x_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.x_reg_ & 0x80);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::LDY(MOS6502& cpu) {
    cpu.y_reg_ = cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.y_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.y_reg_ & 0x80);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::LSR(MOS6502& cpu) {
    uint8_t result = cpu.operand_address_.read() >> 1;
    cpu.setStatusFlag(StatusFlag::CARRY, cpu.operand_address_.read() & 0x01);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x80);
    cpu.operand_address_.write(result);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::NOP(MOS6502& cpu) {
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::ORA(MOS6502& cpu) {
    cpu.accumulator_ |= cpu.operand_address_.read();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::PHA(MOS6502& cpu) {
    cpu.stackPush(cpu.accumulator_);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::PHP(MOS6502& cpu) {
    ProcessorStatus status_to_push = cpu.processor_status_;
    status_to_push.BREAK = 1;
    status_to_push.UNUSED = 1;
    cpu.stackPush(status_to_push.RAW_VALUE);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::PLA(MOS6502& cpu) {
    cpu.accumulator_ = cpu.stackPop();
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::PLP(MOS6502& cpu) {
    ProcessorStatus old_status = cpu.processor_status_;
    cpu.processor_status_.RAW_VALUE = cpu.stackPop();
    cpu.processor_status_.BREAK = old_status.BREAK;
    cpu.processor_status_.UNUSED = old_status.UNUSED;
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::ROL(MOS6502& cpu) {
    uint8_t result = (cpu.operand_address_.read() << 1) | cpu.getStatusFlag(StatusFlag::CARRY);
    cpu.setStatusFlag(StatusFlag::CARRY, cpu.operand_address_.read() & 0x80);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x80);
    cpu.operand_address_.write(result);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::ROR(MOS6502& cpu) {
    uint8_t result = (cpu.operand_address_.read() >> 1) | (cpu.getStatusFlag(StatusFlag::CARRY) << 7);
    cpu.setStatusFlag(StatusFlag::CARRY, cpu.operand_address_.read() & 0x01);
    cpu.setStatusFlag(StatusFlag::ZERO, result == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x80);
    cpu.operand_address_.write(result);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::RTI(MOS6502& cpu) {
    ProcessorStatus old_status = cpu.processor_status_;
    cpu.processor_status_.RAW_VALUE = cpu.stackPop();
    cpu.processor_status_.BREAK = old_status.BREAK;
    cpu.processor_status_.UNUSED = old_status.UNUSED;

    uint16_t pc_low_byte = cpu.stackPop();
    uint16_t pc_high_byte = cpu.stackPop();
    cpu.program_counter_ = (pc_high_byte << 8) | pc_low_byte;
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::RTS(MOS6502& cpu) {
    uint16_t pc_low_byte = cpu.stackPop();
    uint16_t pc_high_byte = cpu.stackPop();
    cpu.program_counter_ = ((pc_high_byte << 8) | pc_low_byte) + 1;
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::SBC(MOS6502& cpu) {
    // Due to the nature of subtraction, it will be subrated one more if carry is cleared
    // So, A = A - memory - (1 - C) = A + -memory - 1 + C
    // Using two's complement: A = A + (~memory + 1) - 1 + C = A + ~memory + C

	// Operating in 16-bit domain to capture carry out
	
	// We can invert the bottom 8 bits with bitwise xor
	uint16_t inverted_operand = static_cast<uint16_t>(cpu.operand_address_.read()) ^ 0x00FF;
	// Notice this is exactly the same as addition from here!
	uint16_t result = static_cast<uint16_t>(cpu.accumulator_) + inverted_operand + static_cast<uint16_t>(cpu.getStatusFlag(StatusFlag::CARRY));
	
    cpu.setStatusFlag(StatusFlag::CARRY, result & 0xFF00);
	cpu.setStatusFlag(StatusFlag::ZERO, (result & 0x00FF) == 0);
	cpu.setStatusFlag(StatusFlag::OVERFLOW_FLAG, (result ^ static_cast<uint16_t>(cpu.accumulator_)) & (result ^ inverted_operand) & 0x0080);
	cpu.setStatusFlag(StatusFlag::NEGATIVE, result & 0x0080);
	
    cpu.accumulator_ = result & 0x00FF;
    return CycleType::ACCEPTS_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::SEC(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::CARRY, 1);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::SED(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::DECIMAL_MODE, 1);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::SEI(MOS6502& cpu) {
    cpu.setStatusFlag(StatusFlag::INTERRUPT_DISABLE, 1);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::STA(MOS6502& cpu) {
    cpu.operand_address_.write(cpu.accumulator_);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::STX(MOS6502& cpu) {
    cpu.operand_address_.write(cpu.x_reg_);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::STY(MOS6502& cpu) {
    cpu.operand_address_.write(cpu.y_reg_);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::TAX(MOS6502& cpu) {
    cpu.x_reg_ = cpu.accumulator_;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.x_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.x_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::TAY(MOS6502& cpu) {
    cpu.y_reg_ = cpu.accumulator_;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.y_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.y_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::TSX(MOS6502& cpu) {
    cpu.x_reg_ = cpu.stack_ptr_;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.x_reg_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.x_reg_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::TXA(MOS6502& cpu) {
    cpu.accumulator_ = cpu.x_reg_;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::TXS(MOS6502& cpu) {
    cpu.stack_ptr_ = cpu.x_reg_;
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::TYA(MOS6502& cpu) {
    cpu.accumulator_ = cpu.y_reg_;
    cpu.setStatusFlag(StatusFlag::ZERO, cpu.accumulator_ == 0);
    cpu.setStatusFlag(StatusFlag::NEGATIVE, cpu.accumulator_ & 0x80);
    return CycleType::NO_ADDITIONAL_CYCLES;
}

MOS6502::CycleType MOS6502::XXX(MOS6502& cpu) {
    // DO NOTHING
    return CycleType::NO_ADDITIONAL_CYCLES;
}

// -------------------- ADDRESSING MODE IMPLEMENTATIONS ------------------------

uint8_t MOS6502::IMP(MOS6502& cpu) {
    cpu.operand_address_ = Pointer::Register::ACCUMULATOR;
    return 0;
}

uint8_t MOS6502::IMM(MOS6502& cpu) {
    cpu.operand_address_ = cpu.program_counter_;
    cpu.program_counter_++;
    return 0;
}

uint8_t MOS6502::ZP0(MOS6502& cpu) {
    cpu.operand_address_ = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;
    return 0;
}

uint8_t MOS6502::ZPX(MOS6502& cpu) {
    cpu.operand_address_ = (cpu.readMemory(cpu.program_counter_) + cpu.x_reg_) & 0x00FF;
    cpu.program_counter_++;
    return 0;
}

uint8_t MOS6502::ZPY(MOS6502& cpu) {
    cpu.operand_address_ = (cpu.readMemory(cpu.program_counter_) + cpu.y_reg_) & 0x00FF;
    cpu.program_counter_++;
    return 0;
}

uint8_t MOS6502::REL(MOS6502& cpu) {
    uint8_t relativeSkip = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;
    cpu.relative_addressing_offset_ = *reinterpret_cast<int8_t*>(&relativeSkip);
    return 0;
}

uint8_t MOS6502::ABS(MOS6502& cpu) {
    uint16_t address_low_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;
    uint16_t address_high_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;

    cpu.operand_address_ = (address_high_byte << 8) | address_low_byte;
    return 0;
}

uint8_t MOS6502::ABX(MOS6502& cpu) {
    uint16_t address_low_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;
    uint16_t address_high_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;

    cpu.operand_address_ = ((address_high_byte << 8) | address_low_byte) + cpu.x_reg_;

    // If page crossed, add 1 more cycle
    if ((std::get<uint16_t>(cpu.operand_address_.get()) & 0xFF00) != (address_high_byte << 8)) {
        return 1;
    }
    return 0;
}

uint8_t MOS6502::ABY(MOS6502& cpu) {
    uint16_t address_low_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;
    uint16_t address_high_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;

    cpu.operand_address_ = ((address_high_byte << 8) | address_low_byte) + cpu.y_reg_;
    
    // If page crossed, add 1 more cycle
    if ((std::get<uint16_t>(cpu.operand_address_.get()) & 0xFF00) != (address_high_byte << 8)) {
        return 1;
    }
    return 0;
}

uint8_t MOS6502::IND(MOS6502& cpu) {
    uint16_t address_low_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;
    uint16_t address_high_byte = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;

    uint16_t target_address = (address_high_byte << 8) | address_low_byte;
    uint16_t indirect_address_low_byte = cpu.readMemory(target_address);
    uint16_t indirect_address_high_byte = cpu.readMemory(target_address + 1);

    // Page Boundary Hardware Bug (page doesn't cross)
    if (address_low_byte == 0x00FF) {
        indirect_address_high_byte = cpu.readMemory(target_address & 0xFF00);
    }

    cpu.operand_address_ = (indirect_address_high_byte << 8) | indirect_address_low_byte;
    return 0;
}

uint8_t MOS6502::IZX(MOS6502& cpu) {
    uint8_t zero_page_adress = cpu.readMemory(cpu.program_counter_) + cpu.x_reg_;
    cpu.program_counter_++;

    uint16_t indirect_address_low_byte = cpu.readMemory(zero_page_adress);
    uint16_t indirect_address_high_byte = cpu.readMemory((zero_page_adress + 1) & 0x00FF);

    cpu.operand_address_ = (indirect_address_high_byte << 8) | indirect_address_low_byte;
    return 0;
}

uint8_t MOS6502::IZY(MOS6502& cpu) {
    uint8_t zero_page_adress = cpu.readMemory(cpu.program_counter_);
    cpu.program_counter_++;

    uint16_t indirect_address_low_byte = cpu.readMemory(zero_page_adress);
    uint16_t indirect_address_high_byte = cpu.readMemory((zero_page_adress + 1) & 0x00FF);

    uint16_t indirect_address = ((indirect_address_high_byte << 8) | indirect_address_low_byte) + cpu.y_reg_;

    cpu.operand_address_ = indirect_address;

    // Page Crossed
    if ((indirect_address & 0xFF00) != (indirect_address_high_byte << 8)) {
        return 1;
    }
    return 0;
}
