#include "nes-sound-sfml.hpp"

NESSoundSFML::NESSoundSFML(const uint32_t& sample_buffer_size): 
    sample_buffer_size_(sample_buffer_size), sample_count_(0), 
    sample_buffer_(std::make_unique<int16_t[]>(sample_buffer_size)),
    sound_buffer_(sample_buffer_.get(), NES_SOUND_SAMPLE_PER_FRAME, 1, NES_SOUND_SAMPLE_RATE, {sf::SoundChannel::Mono}),
    sound_(sound_buffer_) {}

void NESSoundSFML::queueSample(const float& sample) {
    if (sample_count_ >= sample_buffer_size_) return;
    sample_buffer_[sample_count_] = (sample - 0.5f) * UINT16_MAX;
    sample_count_++;
}

void NESSoundSFML::play() {
    sound_.play();
    sample_count_ = 0;
}
