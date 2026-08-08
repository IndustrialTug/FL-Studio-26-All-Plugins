#include "MixerChannel.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace FL {

MixerChannel::MixerChannel(ChannelIdx index, std::string name)
    : m_index(index)
    , m_name(std::move(name))
{}

void MixerChannel::setVolume(float linearGain) noexcept
{
    // Clamp to [0, 4] — 4 corresponds to +12 dB headroom
    m_gain.gain = std::clamp(linearGain, 0.0f, 4.0f);
}

void MixerChannel::setPan(float pan) noexcept
{
    m_gain.pan = std::clamp(pan, -1.0f, 1.0f);
}

void MixerChannel::process(float* buffer, FrameCount frames) noexcept
{
    if (!buffer || frames == 0 || m_muted)
    {
        // When muted, zero the output and update peak meters
        if (buffer && frames > 0)
            std::fill(buffer, buffer + static_cast<size_t>(frames) * 2, 0.0f);
        m_peakL = m_peakR = 0.0f;
        return;
    }

    const float gl = m_gain.leftGain();
    const float gr = m_gain.rightGain();

    float peakL = 0.0f, peakR = 0.0f;

    for (FrameCount i = 0; i < frames; ++i)
    {
        const float l = buffer[i * 2 + 0] * gl;
        const float r = buffer[i * 2 + 1] * gr;

        buffer[i * 2 + 0] = l;
        buffer[i * 2 + 1] = r;

        peakL = std::max(peakL, std::abs(l));
        peakR = std::max(peakR, std::abs(r));
    }

    m_peakL = peakL;
    m_peakR = peakR;
}

void MixerChannel::addSend(ChannelIdx destination, float amount)
{
    std::lock_guard<std::mutex> lock(m_sendsMutex);
    for (auto& s : m_sends)
    {
        if (s.destination == destination)
        {
            s.amount  = amount;
            s.enabled = true;
            return;
        }
    }
    m_sends.push_back({destination, amount, true});
}

void MixerChannel::removeSend(ChannelIdx destination)
{
    std::lock_guard<std::mutex> lock(m_sendsMutex);
    m_sends.erase(
        std::remove_if(m_sends.begin(), m_sends.end(),
            [destination](const Send& s) { return s.destination == destination; }),
        m_sends.end());
}

bool MixerChannel::hasSend(ChannelIdx destination) const
{
    std::lock_guard<std::mutex> lock(m_sendsMutex);
    for (const auto& s : m_sends)
        if (s.destination == destination)
            return true;
    return false;
}

} // namespace FL
