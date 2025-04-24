#ifndef _NES_SOUND_SFML_HPP_
#define _NES_SOUND_SFML_HPP_
// Base Class
#include "nes-sound.hpp"
// Standard Library Headers
#include <cstdint>
#include <memory>
// External Library Headers
#include <SFML/Audio.hpp>
// Project Headers
#include "nes-sound-stream-sfml.hpp"
// Project Defines
#define NES_SOUND_SFML_SAMPLE_BUFFER_SIZE 1024

class NESSoundSFML : public NESSound {
public:
    NESSoundSFML(const uint32_t& sample_buffer_size = NES_SOUND_SFML_SAMPLE_BUFFER_SIZE);

    void queueSample(const float& sample) override;
    void play() override;

private:
    uint32_t sample_count_;
    const uint32_t sample_buffer_size_;
    std::unique_ptr<int16_t[]> sample_buffer_;
    NESSoundStreamSFML sound_stream_;
};

#endif
