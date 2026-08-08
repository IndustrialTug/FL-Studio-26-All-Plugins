#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <cstdint>

namespace fl {

struct MixerChannel {
    std::string name;
    float volume       = 1.0f;
    float panL         = 1.0f;
    float panR         = 1.0f;
    bool  muted        = false;
    float eqLowGainDb  = 0.0f;
    float eqMidGainDb  = 0.0f;
    float eqHighGainDb = 0.0f;
    float compThreshold = 0.7f;
    float compRatio     = 4.0f;
    float compMakeupDb  = 0.0f;
    std::vector<float> bufferL;
    std::vector<float> bufferR;
};

class AudioEngine {
public:
    AudioEngine(int sampleRate = 44100, int bufferSize = 512);

    void processBlock(float* left, float* right, int numSamples);
    void setMasterVolume(float vol);

    int  addChannel(const std::string& name);
    void setPan(int channelIndex, float pan);

    int  sampleRate() const { return m_sampleRate; }
    int  bufferSize()  const { return m_bufferSize; }

private:
    int                          m_sampleRate;
    int                          m_bufferSize;
    float                        m_masterVolume;
    std::atomic<int>             m_activeChannels{0};
    std::vector<MixerChannel>   m_channels;
};

} // namespace fl