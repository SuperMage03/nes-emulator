#include "nes-sound-stream-sfml.hpp"
// File specific constants
static constexpr size_t S_SAMPLE_CHUNK_SIZE = 735;
static constexpr int16_t S_EMPTY_SAMPLE = 0;

NESSoundStreamSFML::NESSoundStreamSFML(): m_chunkQueue(), m_currentSample(0)
{
    initialize(1, 44100, { sf::SoundChannel::Mono });
}

void NESSoundStreamSFML::queueSampleChunk(const int16_t*const& sample_buffer, const size_t& sample_count)
{
    // Invalid
    if (sample_count <= 0) return;

    std::unique_ptr<int16_t[]> chunk_container = std::make_unique<int16_t[]>(S_SAMPLE_CHUNK_SIZE);
    const size_t amount_of_samples_to_load = std::min(sample_count, S_SAMPLE_CHUNK_SIZE);
    memcpy(chunk_container.get(), sample_buffer, amount_of_samples_to_load * sizeof(int16_t));
    
    // Fill in the unallocated chunk space with the last valid sample to create a "fermata" effect, thus removing possible popping sound
    if (amount_of_samples_to_load < S_SAMPLE_CHUNK_SIZE) 
    {
        for (size_t i = amount_of_samples_to_load; i < S_SAMPLE_CHUNK_SIZE; i++) 
        {
            chunk_container[i] = chunk_container[amount_of_samples_to_load - 1];
        }
    }

    // Add the chunk to queue
    m_chunkQueue.emplace(std::move(chunk_container));
}

bool NESSoundStreamSFML::onGetData(Chunk& data)
{
    // number of samples to stream every time the function is called;
    // in a more robust implementation, it should be a fixed
    // amount of time rather than an arbitrary number of samples
    const int samplesToStream = S_SAMPLE_CHUNK_SIZE;

    // End of current chunk reached
    if (m_currentSample + samplesToStream > S_SAMPLE_CHUNK_SIZE)
    {
        switch (m_chunkQueue.size()) 
        {
        // When the queue is empty, just stay silent by playing the empty sample
        case 0:
            data.samples = &S_EMPTY_SAMPLE;
            data.sampleCount = 1;
            m_currentSample = S_SAMPLE_CHUNK_SIZE;
            break;
        // When NES hasn't queued the next chunk of samples, we keep playing the last part of the sample current chunk
        //   (Making a "fermata" with the last sample in current chunk instead of pausing to avoid popping noises)
        case 1:
            data.samples = &m_chunkQueue.front()[S_SAMPLE_CHUNK_SIZE - 1];
            data.sampleCount = 1;
            m_currentSample = S_SAMPLE_CHUNK_SIZE;
            break;
        // We have chunks in queue, remove the current chunk and starting playing the next chunk in queue
        default:
            m_chunkQueue.pop();
            data.samples = &m_chunkQueue.front()[0];
            data.sampleCount = samplesToStream;
            m_currentSample = samplesToStream;
            break;
        }
        // Return true to signal SFML we are not stopping the stream
        return true;
    }

    // end not reached: stream the current chunk and continue
    data.samples = &m_chunkQueue.front()[m_currentSample];
    data.sampleCount = samplesToStream;
    m_currentSample += samplesToStream;
    // Return true to signal SFML we are not stopping the stream
    return true;
}

void NESSoundStreamSFML::onSeek(sf::Time timeOffset)
{
    // compute the corresponding sample index according to the sample rate and channel count
    m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
}
