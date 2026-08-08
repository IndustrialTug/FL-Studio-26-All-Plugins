// StepSequencer.h — 16-step pattern sequencer header

#pragma once
#include <vector>
#include <string>

struct SequencerChannel { std::string name; std::vector<bool> steps; float volume = 1.0f; };

class StepSequencer {
public:
    StepSequencer(int steps, int bpm, int sampleRate);
    void toggleStep(int channel, int step);
    void setBPM(int bpm);
    int  samplesPerStep() const;
    bool advance();
    std::vector<int> getActiveChannels() const;
    int  addChannel(const std::string& name);
    int  getCurrentStep() const { return m_currentStep; }
private:
    int m_steps, m_bpm, m_sampleRate, m_currentStep;
    std::vector<SequencerChannel> m_channels;
};
