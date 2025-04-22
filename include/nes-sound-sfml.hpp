#ifndef _NES_SOUND_SFML_HPP_
#define _NES_SOUND_SFML_HPP_
#include "nes-sound.hpp"
// Standard Library Headers
#include <cstdint>
#include <memory>
// External Library Headers
#include <SFML/Audio.hpp>
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
    sf::SoundBuffer sound_buffer_;
    sf::Sound sound_;
};

#endif
