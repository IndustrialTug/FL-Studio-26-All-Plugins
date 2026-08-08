#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace fl {

class StepSequencer {
public:
    struct Channel { std::vector<bool> steps; };
    StepSequencer(int steps, int bpm, int sampleRate);
    void toggleStep(int channel, int step);
    void  setBPM(int bpm);
    int   samplesPerStep() const;
    bool  advance();
    std::vector<int> getActiveChannels() const;
    int   bpm()       const { return m_bpm; }
    int   currentStep() const { return m_currentStep; }
    int   stepCount()  const { return m_steps; }

private:
    int                   m_steps;
    int                   m_bpm;
    int                   m_sampleRate;
    int                   m_currentStep = 0;
    std::vector<Channel>  m_channels;
};

} // namespace fl