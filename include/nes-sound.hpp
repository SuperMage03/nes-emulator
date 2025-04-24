#ifndef _NES_SOUND_HPP_
#define _NES_SOUND_HPP_
// Project Defines
// 44100 samples per second
#define NES_SOUND_SAMPLE_RATE 44100
// 60 FPS for NES
#define NES_SOUND_FRAME_PER_SECOND 60
// NES_SOUND_SAMPLE_RATE / NES_SOUND_FRAME_PER_SECOND
#define NES_SOUND_SAMPLE_PER_FRAME 735

class NESSound {
public:
    // Destructor
    virtual ~NESSound() = default;

    /**
    * @brief  Queues sample
    * @param  sample: sample value
    * @return None
    */
    virtual void queueSample(const float& sample) = 0;

    /**
    * @brief  Plays sound
    * @param  None
    * @return None
    */
    virtual void play() = 0;
};

#endif
