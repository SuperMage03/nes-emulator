#include "nes-sound-sfml.hpp"

NESSoundSFML::NESSoundSFML(const uint32_t& sample_buffer_size): 
    sample_buffer_size_(sample_buffer_size), sample_count_(0), 
    sample_buffer_(std::make_unique<int16_t[]>(sample_buffer_size)),
    sound_stream_() {}

void NESSoundSFML::queueSample(const float& sample) {
    if (sample_count_ >= sample_buffer_size_) return;
    // float sample (from 0.0f to 1.0f) to 16 bit PCM sample
    int16_t converted_sample = sample * 32767;
    sample_buffer_[sample_count_] = converted_sample;
    sample_count_++;
}

void NESSoundSFML::play() {
    sound_stream_.queueSampleChunk(sample_buffer_.get(), sample_count_);
    sample_count_ = 0;
    if (sound_stream_.getStatus() != sf::SoundSource::Status::Playing) {
        sound_stream_.play();
    }
}
