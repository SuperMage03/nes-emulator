#include "nes-sound-sfml.hpp"
#include <stdexcept>

NESSoundSFML::NESSoundSFML(const uint32_t& sample_buffer_size): 
    sample_buffer_size_(sample_buffer_size), sample_count_(0), 
    sample_buffer_(std::make_unique<int16_t[]>(sample_buffer_size)),
    sound_buffer_(),
    sound_(sound_buffer_) {}

void NESSoundSFML::queueSample(const float& sample) {
    if (sample_count_ >= sample_buffer_size_) return;
    // float sample to 16 bit PCM sample
    int16_t converted_sample = sample * 32767;
    sample_buffer_[sample_count_] = converted_sample;
    sample_count_++;
}

void NESSoundSFML::play() {
    if (!sound_buffer_.loadFromSamples(sample_buffer_.get(), sample_count_, 1, NES_SOUND_SAMPLE_RATE, {sf::SoundChannel::Mono})) {
        throw std::runtime_error{"Fail to load samples to sound buffer!"};
    }
    sound_.play();
    sample_count_ = 0;
}
