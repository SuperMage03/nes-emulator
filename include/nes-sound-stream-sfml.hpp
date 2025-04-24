#ifndef _NES_SOUND_STREAM_SFML_HPP_
#define _NES_SOUND_STREAM_SFML_HPP_
// Base Class
#include <SFML/Audio.hpp>
// Standard Library Headers
#include <memory>
#include <queue>

class NESSoundStreamSFML : public sf::SoundStream {
public:
    NESSoundStreamSFML();
    void queueSampleChunk(const int16_t* const& sample_buffer, const size_t& sample_count);
private:
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;
    // For managing audio streaming
    std::queue<std::unique_ptr<int16_t[]>> m_chunkQueue;
    // For seeking
    std::size_t                         m_currentSample;
};

#endif
