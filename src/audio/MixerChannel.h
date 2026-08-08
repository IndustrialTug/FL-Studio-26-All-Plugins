#pragma once
#include "../../include/FLTypes.h"
#include <string>
#include <vector>
#include <mutex>

namespace FL {

// ---------------------------------------------------------------------------
// MixerChannel
// Represents a single channel strip in the mixer: gain, pan, mute, solo,
// send routing, and per-channel peak metering.
// ---------------------------------------------------------------------------
class MixerChannel {
public:
    explicit MixerChannel(ChannelIdx index, std::string name = "");
    ~MixerChannel() = default;

    // Non-copyable, movable
    MixerChannel(const MixerChannel&)            = delete;
    MixerChannel& operator=(const MixerChannel&) = delete;
    MixerChannel(MixerChannel&&)                 = default;
    MixerChannel& operator=(MixerChannel&&)      = default;

    // --- Identity ---
    ChannelIdx       getIndex() const noexcept { return m_index; }
    const std::string& getName() const noexcept { return m_name; }
    void               setName(std::string name) { m_name = std::move(name); }

    // --- Volume (linear gain, [0, 4]) ---
    float getVolume() const noexcept { return m_gain.gain; }
    void  setVolume(float linearGain) noexcept;

    // --- Pan ([-1, +1], 0 = centre) ---
    float getPan() const noexcept { return m_gain.pan; }
    void  setPan(float pan) noexcept;

    // --- Mute / solo ---
    bool isMuted() const noexcept { return m_muted; }
    void setMuted(bool muted) noexcept { m_muted = muted; }
    void toggleMute() noexcept { m_muted = !m_muted; }

    bool isSoloed() const noexcept { return m_soloed; }
    void setSoloed(bool soloed) noexcept { m_soloed = soloed; }

    // --- Processing ---
    // Process buffer in-place.  Applies volume, pan, and mute.
    // buffer: interleaved stereo float samples (frames * 2 floats).
    void process(float* buffer, FrameCount frames) noexcept;

    // --- Send routing ---
    // A send routes a scaled copy of this channel's signal to another channel.
    struct Send {
        ChannelIdx destination = 0;
        float      amount      = 1.0f;  // linear send level
        bool       enabled     = true;
    };

    void addSend(ChannelIdx destination, float amount = 1.0f);
    void removeSend(ChannelIdx destination);
    bool hasSend(ChannelIdx destination) const;
    const std::vector<Send>& getSends() const noexcept { return m_sends; }

    // --- Peak metering ---
    // Updated by process().  Values are linear peak over the last buffer.
    float peakLeft()  const noexcept { return m_peakL; }
    float peakRight() const noexcept { return m_peakR; }
    void  resetPeaks() noexcept { m_peakL = m_peakR = 0.0f; }

private:
    ChannelIdx        m_index;
    std::string       m_name;
    GainPan           m_gain;
    bool              m_muted  = false;
    bool              m_soloed = false;
    std::vector<Send> m_sends;
    mutable std::mutex m_sendsMutex;

    float             m_peakL = 0.0f;
    float             m_peakR = 0.0f;
};

} // namespace FL
