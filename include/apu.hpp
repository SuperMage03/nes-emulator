#ifndef _APU_HPP_
#define _APU_HPP_
// Standard Library Headers
#include <array>
#include <cstdint>
// Project Headers
#include "nes-sound.hpp"

class APU {
public:
    struct Timer {
        Timer();
        void clock();
        // Members
        uint16_t value;
        uint16_t period;
    };

    struct LengthCounter {
    private:
        static const std::array<uint8_t, 0x20> s_load_table;
    public:
        // Methods
        LengthCounter();
        void load(const uint8_t& load_table_index);
        void clock();
        // Members
        bool is_enabled;
        uint8_t value;
    };

    struct Envelope {
        void clock();
        // Members
        bool is_enabled;
        bool is_starting;
        bool is_looping;
        uint8_t divider_period;
        uint8_t divider_value;
        uint8_t volume;
    };

    struct Sweep {
        void clock();
        // Members
        union {
            uint8_t state_value;
            struct {
                uint8_t shift_count : 3;
                uint8_t is_negated : 1;
                uint8_t period : 3;
                uint8_t is_enabled : 1;
            };
        };
        bool is_reloading;
        uint8_t value;
    };

    class PulseChannel {
    public:
        bool is_enabled_;
        // Channel Specific Variable
        uint8_t duty_cycle_;
        uint8_t duty_value_;

        Timer timer_;
        LengthCounter length_counter_;
        Envelope envelope_;
        Sweep sweep_;

        // Methods
        explicit PulseChannel(void (*timer_period_modifier)(uint16_t& timer_reload, const uint16_t& change));
        void clockTimer();
        void clockSweep();
        uint8_t getOutput() const;

    private:
        static const std::array<std::array<uint8_t, 8>, 4> s_duty_value_table;
        void (*timer_period_modifier_)(uint16_t& timer_reload, const uint16_t& change);

    };

    struct TriangleChannel {
        LengthCounter length_counter_;
    } triangle_channel;

    struct NoiseChannel {
        LengthCounter length_counter_;
        Envelope envelope_;
    } noise_channel;

    struct DMCChannel {
        LengthCounter length_counter_;
    } dmc_channel;

    // Read APU Register
    uint8_t readAPURegister(const uint8_t& address);
    // Write APU Register
    bool writeAPURegister(const uint8_t& address, const uint8_t& data);
    void clockTimers();
    void clockLengthCounters();
    void clockEnvelopes();
    void clockSweeps();


    void clockSequencer();
    // run APU Cycle
    void clockAPU();

    /**
     * @brief  Connects Sound System
     * @param  sound_system: sound system to connect to
     * @return None
    */
    void connectSoundSystem(NESSound& sound_system);

    // Mixer Functions
    float samplePulseOut(const uint8_t& pulse_1, const uint8_t& pulse_2) const;
    float sampleTNDOut(const uint8_t& triangle, const uint8_t& noise, const uint8_t& dmc) const;
    float sampleMixerOut(const uint8_t& pulse_1, const uint8_t& pulse_2, const uint8_t& triangle, const uint8_t& noise, const uint8_t& dmc) const;

    // Helper Function
    float sampleSquarePulseWave(const float& frequency, const float& time);

    static void pulseOneTimerPeriodNegateModifier(uint16_t& timer_reload, const uint16_t& change);
    static void pulseTwoTimerPeriodNegateModifier(uint16_t& timer_reload, const uint16_t& change);

private:
    // Mixer Lookup tables
    static const std::array<float, 31> pulse_table;
    static const std::array<float, 203> tnd_table;

    uint64_t clock_count_;
    
    // Sequencer Value
    uint8_t sequencer_value_;
    enum class SequencerMode {
        FourStep,
        FiveStep,
    } sequencer_mode_;
    
    bool irq_inhibit_;
    bool frame_irq_;
    bool irq_requested_;

    NESSound* sound_system_;
    
    PulseChannel pulse_1_channel{pulseOneTimerPeriodNegateModifier};
    PulseChannel pulse_2_channel{pulseTwoTimerPeriodNegateModifier};
};

#endif
