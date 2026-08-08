#include "flstudio/sequencer/StepSequencer.h"
#include <algorithm>

namespace fl {

StepSequencer::StepSequencer(int steps, int bpm, int sampleRate)
    : m_steps(steps), m_bpm(bpm), m_sampleRate(sampleRate)
{
    m_channels.resize(8);
    for (auto& ch : m_channels) ch.steps.assign(steps, false);
}

void StepSequencer::toggleStep(int channel, int step)
{
    if (channel < (int)m_channels.size() && step < m_steps)
        m_channels[channel].steps[step] = !m_channels[channel].steps[step];
}

void StepSequencer::setBPM(int bpm)
{
    m_bpm = std::max(20, std::min(999, bpm));
}

int StepSequencer::samplesPerStep() const
{
    double stepSeconds = 60.0 / m_bpm / 4.0;
    return static_cast<int>(stepSeconds * m_sampleRate);
}

bool StepSequencer::advance()
{
    m_currentStep = (m_currentStep + 1) % m_steps;
    return m_currentStep == 0;
}

std::vector<int> StepSequencer::getActiveChannels() const
{
    std::vector<int> active;
    for (int i = 0; i < (int)m_channels.size(); ++i)
        if (m_channels[i].steps[m_currentStep])
            active.push_back(i);
    return active;
}

} // namespace fl