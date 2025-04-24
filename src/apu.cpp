#include "apu.hpp"
#include "nes-sound.hpp"
// Standard Library Headers
#include <cmath>
#include <cstdint>
#include <numbers>

// From NESDev Wiki: pulse_table [n] = 95.52 / (8128.0 / n + 100)
const std::array<float, 31> APU::pulse_table = {{
    0.0f, 0.01160913985222578f, 0.02293948084115982f, 0.0340009480714798f, 0.04480300098657608f, 0.055354658514261246f, 0.06566452980041504f, 0.07574082165956497f,  
    0.0855913981795311f, 0.09522374719381332f, 0.10464505106210709f, 0.11386215686798096f, 0.12288164347410202f, 0.13170979917049408f, 0.14035265147686005f, 0.14881595969200134f,  
    0.15710526704788208f, 0.16522587835788727f, 0.1731829196214676f, 0.18098124861717224f, 0.18862558901309967f, 0.1961204558610916f, 0.20347018539905548f, 0.21067893505096436f,  
    0.21775075793266296f, 0.22468949854373932f, 0.23149888217449188f, 0.23818248510360718f, 0.24474377930164337f, 0.2511860728263855f, 0.25751256942749023f,    
}};

// From NESDev Wiki: tnd_table [n] = 163.67 / (24329.0 / n + 100)
const std::array<float, 203> APU::tnd_table = {{
    0.0f, 0.006699823774397373f, 0.013345019891858101f, 0.019936254248023033f, 0.026474179700016975f, 0.03295944258570671f, 0.03939267620444298f, 0.04577450081706047f,  
    0.052105534821748734f, 0.05838638171553612f, 0.06461763381958008f, 0.07079987227916718f, 0.07693368196487427f, 0.08301962912082672f, 0.08905825763940811f, 0.0950501337647438f,  
    0.10099579393863678f, 0.10689576715230942f, 0.1127505823969841f, 0.11856075376272202f, 0.12432678788900375f, 0.1300491839647293f, 0.13572844862937927f, 0.14136505126953125f,  
    0.14695948362350464f, 0.15251220762729645f, 0.1580236852169037f, 0.16349439322948456f, 0.16892476379871368f, 0.17431525886058807f, 0.17966629564762115f, 0.18497830629348755f,  
    0.19025173783302307f, 0.19548699259757996f, 0.20068448781967163f, 0.20584462583065033f, 0.2109678089618683f, 0.21605443954467773f, 0.22110490500926971f, 0.22611959278583527f,  
    0.23109887540340424f, 0.23604312539100647f, 0.24095271527767181f, 0.24582800269126892f, 0.25066936016082764f, 0.2554771304130554f, 0.26025164127349854f, 0.264993280172348f,  
    0.26970234513282776f, 0.2743792235851288f, 0.2790241837501526f, 0.2836375832557678f, 0.28821972012519836f, 0.2927709221839905f, 0.2972915470600128f, 0.3017818331718445f,  
    0.30624210834503174f, 0.31067267060279846f, 0.3150738477706909f, 0.3194459080696106f, 0.323789119720459f, 0.32810378074645996f, 0.3323901891708374f, 0.3366486132144928f,  
    0.34087929129600525f, 0.345082551240921f, 0.3492586314678192f, 0.35340777039527893f, 0.35753026604652405f, 0.36162638664245605f, 0.3656963109970093f, 0.36974036693573f,  
    0.3737587630748749f, 0.37775173783302307f, 0.38171955943107605f, 0.3856624364852905f, 0.389580637216568f, 0.3934743404388428f, 0.39734384417533875f, 0.40118929743766785f,  
    0.40501096844673157f, 0.408809095621109f, 0.4125838577747345f, 0.41633546352386475f, 0.42006415128707886f, 0.42377012968063354f, 0.4274536073207855f, 0.4311147630214691f,  
    0.4347538352012634f, 0.4383710026741028f, 0.4419664442539215f, 0.4455403983592987f, 0.4490930140018463f, 0.45262452960014343f, 0.4561350643634796f, 0.45962488651275635f,  
    0.4630941152572632f, 0.46654295921325684f, 0.46997156739234924f, 0.4733801782131195f, 0.4767689108848572f, 0.48013797402381897f, 0.4834875166416168f, 0.48681768774986267f,  
    0.4901287257671356f, 0.49342072010040283f, 0.496693879365921f, 0.49994832277297974f, 0.5031842589378357f, 0.5064018368721008f, 0.5096011757850647f, 0.5127824544906616f,  
    0.5159458518028259f, 0.5190914869308472f, 0.5222194790840149f, 0.5253300666809082f, 0.5284232497215271f, 0.5314993262290955f, 0.5345584154129028f, 0.5376005172729492f,  
    0.5406259298324585f, 0.5436347126960754f, 0.5466270446777344f, 0.5496030449867249f, 0.5525628328323364f, 0.5555065274238586f, 0.5584343075752258f, 0.5613462328910828f,  
    0.5642424821853638f, 0.5671232342720032f, 0.569988489151001f, 0.5728384256362915f, 0.5756732225418091f, 0.5784929394721985f, 0.5812976956367493f, 0.584087610244751f,  
    0.5868628025054932f, 0.5896234512329102f, 0.5923696160316467f, 0.5951013565063477f, 0.597818911075592f, 0.6005222797393799f, 0.6032115817070007f, 0.6058870553970337f,  
    0.608548641204834f, 0.6111965179443359f, 0.6138308048248291f, 0.616451621055603f, 0.6190590262413025f, 0.621653139591217f, 0.6242340207099915f, 0.6268018484115601f,  
    0.6293566823005676f, 0.6318985819816589f, 0.6344277262687683f, 0.6369441747665405f, 0.6394480466842651f, 0.6419393420219421f, 0.6444182395935059f, 0.6468848586082458f,  
    0.6493391990661621f, 0.6517813801765442f, 0.6542115807533264f, 0.656629741191864f, 0.6590360403060913f, 0.6614305973052979f, 0.6638134121894836f, 0.666184663772583f,  
    0.668544352054596f, 0.670892596244812f, 0.673229455947876f, 0.6755550503730774f, 0.677869439125061f, 0.6801727414131165f, 0.6824649572372437f, 0.6847462058067322f,  
    0.6870166063308716f, 0.6892762184143066f, 0.6915251016616821f, 0.6937633156776428f, 0.6959909200668335f, 0.6982080936431885f, 0.700414776802063f, 0.7026111483573914f,  
    0.7047972083091736f, 0.7069730758666992f, 0.709138810634613f, 0.7112944722175598f, 0.7134401202201843f, 0.7155758738517761f, 0.71770179271698f, 0.7198178768157959f,  
    0.7219242453575134f, 0.7240209579467773f, 0.7261080741882324f, 0.7281856536865234f, 0.7302538156509399f, 0.7323125600814819f, 0.734362006187439f, 0.736402153968811f,  
    0.7384330630302429f, 0.740454912185669f, 0.7424675822257996f,    
}};

APU::Timer::Timer(): value(0), period(0) {}

void APU::Timer::clock() {
    if (value > 0) {
        value--;
        return;
    }
    value = period;
}

// From NES Dev Wiki
const std::array<uint8_t, 0x20> APU::LengthCounter::s_load_table = {{
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
}};

APU::LengthCounter::LengthCounter(): is_enabled(false), value(0) {}

void APU::LengthCounter::load(const uint8_t& load_table_index) {
    value = s_load_table.at(load_table_index);
}

void APU::LengthCounter::clock() {
    if (is_enabled && (value > 0)) {
        value--;
    }
}

void APU::Envelope::clock() {
    if (is_starting) {
        volume = 0x0F;
        divider_value = divider_period;
        is_starting = false;
        return;
    }
    
    if (divider_value == 0) {
        if (volume == 0) {
            volume = is_looping ? 0x0F : 0;
        }
        else {
            volume--;
        }
        divider_value = divider_period;
    }
    else {
        divider_value--;
    }
}

const std::array<std::array<uint8_t, 8>, 4> APU::PulseChannel::s_duty_value_table = {{
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1},
}};

APU::PulseChannel::PulseChannel(void (*timer_period_modifier_)(uint16_t& timer_reload, const uint16_t& change)): 
    timer_period_modifier_(timer_period_modifier_) {}

void APU::PulseChannel::clockTimer() {
    if (timer_.value == 0) {
        duty_value_ = (duty_value_ + 1) % 8;
    }
    timer_.clock();
}

void APU::PulseChannel::clockSweep() {
    if (sweep_.is_reloading) {
        sweep_.value = sweep_.period + 1;
        sweep_.is_reloading = false;
        return;
    }

    if (sweep_.value > 0) {
        sweep_.value--;
        return;
    }

    sweep_.value = sweep_.period + 1;
    if (!sweep_.is_enabled) {
        return;
    }

    const uint16_t change = timer_.period >> sweep_.shift_count;
    if (!sweep_.is_negated) {
        timer_.period += change;
        return;
    }
    timer_period_modifier_(timer_.period, change);
}

uint8_t APU::PulseChannel::getOutput() const {
    const uint8_t is_active = s_duty_value_table.at(duty_cycle_).at(duty_value_);

    if (is_active) {
        // asd
    }

    // Stay Silent
    if (!is_enabled_ || !is_active || (length_counter_.value == 0) || (timer_.value < 8) || (timer_.period > 0x07FF)) {
        return 0;
    }
    
    return envelope_.is_enabled ? envelope_.volume : envelope_.divider_period;
}

// From NES Dev Wiki
const std::array<uint8_t, 32> APU::TriangleChannel::s_duty_value_table = { {
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
} };

void APU::TriangleChannel::clockTimer() {
    if (timer_.value > 0) {
        timer_.value--;
        return;
    }
    // Reload Timer
    timer_.value = timer_.period + 1;
    if ((length_counter_.value > 0) && (linear_counter_.value > 0)) {
        duty_value_ = (duty_value_ + 1) % 32;
    }
}

void APU::TriangleChannel::clockLengthCounter() {
    if (length_counter_.is_enabled && (length_counter_.value > 0)) {
        length_counter_.value--;
    }
}

void APU::TriangleChannel::clockLinearCounter() {
    if (linear_counter_.value > 0) {
        linear_counter_.value--;
        return;
    }

    if (linear_counter_.is_reloading) {
        linear_counter_.value = linear_counter_.period;
    }

    if (!linear_counter_.control_flag) {
        linear_counter_.is_reloading = false;
    }
}

uint8_t APU::TriangleChannel::getOutput() const {
    if (!is_enabled_) {
        return 0;
    }
    if (length_counter_.value == 0) {
        return 0;
    }
    if (linear_counter_.value == 0) {
        return 0;
    }
    return s_duty_value_table.at(duty_value_);
}

uint8_t APU::readAPURegister(const uint8_t& address) {
    switch (address) {
    case 0x15:
        break;
    }
    return 0;
}

bool APU::writeAPURegister(const uint8_t& address, const uint8_t& data) {
    switch (address) {
    // Done
    case 0x00:
        pulse_1_channel.duty_cycle_ = data >> 6;
        pulse_1_channel.length_counter_.is_enabled = (data & 0b00100000) == 0;
        pulse_1_channel.envelope_.is_looping = data & 0b00100000;
        pulse_1_channel.envelope_.is_enabled = (data & 0b00010000) == 0;
        pulse_1_channel.envelope_.divider_period = data & 0x0F;
        pulse_1_channel.envelope_.volume = data & 0x0F;
        pulse_1_channel.envelope_.is_starting = true;
        // noise_channel.envelope.setLooping(data & 0b00100000);
        // noise_channel.envelope.setStart(true);
        break;
    // Done
    case 0x01:
        pulse_1_channel.sweep_.state_value = data;
        pulse_1_channel.sweep_.is_reloading = true;
        break;
    // Done
    case 0x02:
        pulse_1_channel.timer_.period = (pulse_1_channel.timer_.period & 0xFF00) | data;
        break;
    // Done
    case 0x03:
        pulse_1_channel.timer_.period = (pulse_1_channel.timer_.period & 0x00FF) | ((data & 0x07) << 8);
        pulse_1_channel.length_counter_.load(data >> 3);
        pulse_1_channel.envelope_.is_starting = true;
        pulse_1_channel.duty_value_ = 0;
        break;
    // Done
    case 0x04:
        pulse_2_channel.duty_cycle_ = data >> 6;
        pulse_2_channel.length_counter_.is_enabled = (data & 0b00100000) == 0;
        pulse_2_channel.envelope_.is_looping = data & 0b00100000;
        pulse_2_channel.envelope_.is_enabled = (data & 0b00010000) == 0;
        pulse_2_channel.envelope_.divider_period = data & 0x0F;
        pulse_2_channel.envelope_.volume = data & 0x0F;
        pulse_2_channel.envelope_.is_starting = true;
        // noise_channel.envelope.setLooping(data & 0b00100000);
        // noise_channel.envelope.setStart(true);
        break;
    // Done
    case 0x05:
        pulse_2_channel.sweep_.state_value = data;
        pulse_2_channel.sweep_.is_reloading = true;
        break;
    // Done
    case 0x06:
        pulse_2_channel.timer_.period = (pulse_2_channel.timer_.period & 0xFF00) | data;
        break;
    // Done
    case 0x07:
        pulse_2_channel.timer_.period = (pulse_2_channel.timer_.period & 0x00FF) | ((data & 0x07) << 8);
        pulse_2_channel.length_counter_.load(data >> 3);
        pulse_2_channel.envelope_.is_starting = true;
        pulse_2_channel.duty_value_ = 0;
        break;
    // Triangle Channel Control
    case 0x08:
        triangle_channel.length_counter_.is_enabled = data & 0b10000000;
        triangle_channel.linear_counter_.control_flag = data & 0b10000000;
        triangle_channel.linear_counter_.period = data & 0b01111111;
        break;
    // Triangle Channel Timer Low Byte
    case 0x0A:
        triangle_channel.timer_.period = (triangle_channel.timer_.period & 0xFF00) | data;
        break;
    // Triangle Channel Timer High Byte and Length Counter value
    case 0x0B:
        triangle_channel.timer_.period = (triangle_channel.timer_.period & 0x00FF) | ((data & 0x07) << 8);
        triangle_channel.timer_.value = triangle_channel.timer_.period + 1;
        triangle_channel.length_counter_.load(data >> 3);
        triangle_channel.linear_counter_.is_reloading = true;
        break;
    // TODO
    case 0x15:
        pulse_1_channel.is_enabled_ = data & 0b00000001;
        pulse_2_channel.is_enabled_ = data & 0b00000010;
        triangle_channel.is_enabled_ = data & 0b00000100;
        // noise_channel.is_enabled_ = data & 0b00001000;
        // dmc_channel.is_enabled_ = data & 0b00010000;

        if (!pulse_1_channel.is_enabled_) {
            pulse_1_channel.length_counter_.value = 0;
        }
        if (!pulse_2_channel.is_enabled_) {
            pulse_2_channel.length_counter_.value = 0;
        }
        if (!triangle_channel.is_enabled_) {
            triangle_channel.length_counter_.value = 0;
        }
        // if (!noise_channel.is_enabled_) {
        //     noise_channel.length_counter_.value = 0;
        // }
        // if (!dmc_channel.is_enabled_) {
        //     dmc_channel.length_counter_.value = 0;
        // }
        break;
    case 0x17:
        sequencer_mode_ = static_cast<APU::SequencerMode>((data & 0b10000000) > 0);
        frame_irq_ = (data & 0b01000000) > 0;
        break;
    }
    return true;
}

void APU::clockTimers() {
    triangle_channel.clockTimer();
    if (clock_count_ % 2) {
        pulse_1_channel.clockTimer();
        pulse_2_channel.clockTimer();
    //   this->chan.noise.timer_clock();
    //   this->chan.dmc.timer_clock(this->mem, this->interrupt);
    }
}

void APU::clockLengthCounters() {
    pulse_1_channel.length_counter_.clock();
    pulse_2_channel.length_counter_.clock();
    // noise_channel.length_counter_.clock();
    triangle_channel.clockLengthCounter();
}

void APU::clockEnvelopes() {
    pulse_1_channel.envelope_.clock();
    pulse_2_channel.envelope_.clock();
    // noise_channel.envelope_.clock();
    triangle_channel.clockLinearCounter();
}

void APU::clockSweeps() {
    pulse_1_channel.clockSweep();
    pulse_2_channel.clockSweep();
}

void APU::clockSequencer() {
    sequencer_value_++;

    switch (sequencer_mode_) {
    case SequencerMode::FourStep: {
        sequencer_value_ %= 4;
        switch (sequencer_value_) {
        case 0:
            clockEnvelopes();
            break;
        case 1:
            clockSweeps();
            clockLengthCounters();
            break;
        case 2:
            clockEnvelopes();
            break;
        case 3:
            clockSweeps();
            clockLengthCounters();
            if (!irq_inhibit_) {
                frame_irq_ = true;
            }
            break;
        }
    }
    break;

    case SequencerMode::FiveStep: {
        sequencer_value_ %= 5;
        switch (sequencer_value_) {
            case 0:
                clockEnvelopes();
                break;
            case 1:
                clockEnvelopes();
                clockSweeps();
                clockLengthCounters();
                break;
            case 2:
                clockEnvelopes();
                break;
            case 3:
                clockEnvelopes();
                clockSweeps();
                clockLengthCounters();
                break;
        }
    }
    break;
    }
}

void APU::clockAPU() {
    clock_count_++;
    clockTimers();

    // For more accurate clocking
    uint64_t previous_clcok_count = clock_count_ - 1;

    // APU is clocked every CPU clock (1789773Hz) and Sequencer is clocked at 240Hz
    //   So sequncer clocks every 1789773 / 240 = 7457.3875 apu cycles
    if (static_cast<uint64_t>(previous_clcok_count / 7457.3875f) != static_cast<uint64_t>(clock_count_ / 7457.3875f)) {
        clockSequencer();
    }

    // Sample Rate is 44100Hz, so similar to clocking sequencer
    //   We will sample every 1789773 / 44100 = 40.5844217687 apu cycles
    if (static_cast<uint64_t>(previous_clcok_count / 40.5844217687f) != static_cast<uint64_t>(clock_count_ / 40.5844217687f)) {
        if (sound_system_) {
            float sampleOut = sampleMixerOut(pulse_1_channel.getOutput(), pulse_2_channel.getOutput(), triangle_channel.getOutput(), 0, 0);
            sound_system_->queueSample(sampleOut);
        }
    }

    // TODO: Add DMC IRQ
    if (frame_irq_) {
        irq_requested_ = true;
    }
}

void APU::connectSoundSystem(NESSound& sound_system) {
    sound_system_ = &sound_system;
}

float APU::samplePulseOut(const uint8_t& pulse_1, const uint8_t& pulse_2) const {
    return pulse_table.at(pulse_1 + pulse_2);
}

float APU::sampleTNDOut(const uint8_t& triangle, const uint8_t& noise, const uint8_t& dmc) const {
    return tnd_table.at(3 * triangle + 2 * noise + dmc);
}

float APU::sampleMixerOut(const uint8_t& pulse_1, const uint8_t& pulse_2, const uint8_t& triangle, const uint8_t& noise, const uint8_t& dmc) const {
    return samplePulseOut(pulse_1, pulse_2) + sampleTNDOut(triangle, noise, dmc);
}

float APU::sampleSquarePulseWave(const float& frequency, const float& time) {
    const float duty_cycle = 0.0f;
    const uint8_t harmonics = 15;

    const float phase = 2.0f * std::numbers::pi_v<float> * duty_cycle;

    uint8_t x = 0x00;

    float sine_wave_a = 0.0f;
    for (uint8_t i = 1; i < harmonics; i++) {
        sine_wave_a += std::sinf(x * frequency * (2 * std::numbers::pi_v<float>) * i) / i;
    }

    float sine_wave_b = 0.0f;
    for (uint8_t i = 1; i < harmonics; i++) {
        sine_wave_b += std::sinf((x * frequency - phase) * (2 * std::numbers::pi_v<float>) * i) / i;
    }

    return sine_wave_a - sine_wave_b;
}

void APU::pulseOneTimerPeriodNegateModifier(uint16_t &timer_reload, const uint16_t &change) {
    timer_reload -= change + 1;
}

void APU::pulseTwoTimerPeriodNegateModifier(uint16_t &timer_reload, const uint16_t &change) {
    timer_reload -= change;
}
