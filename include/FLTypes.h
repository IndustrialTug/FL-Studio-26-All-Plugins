#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <functional>

namespace FL {

// ---------------------------------------------------------------------------
// Primitive aliases
// ---------------------------------------------------------------------------
using SampleRate  = uint32_t;
using FrameCount  = uint32_t;
using ChannelIdx  = uint16_t;
using NoteNumber  = uint8_t;   // 0-127 MIDI note
using Velocity    = uint8_t;   // 0-127 MIDI velocity
using PluginId    = uint32_t;
using PatternId   = uint32_t;
using TrackId     = uint32_t;
using Tick        = int64_t;   // ticks since song start (PPQ-relative)

// ---------------------------------------------------------------------------
// Audio constants
// ---------------------------------------------------------------------------
constexpr SampleRate kDefaultSampleRate   = 44100;
constexpr FrameCount kDefaultBufferFrames = 512;
constexpr size_t     kMaxMixerChannels    = 128;
constexpr size_t     kMaxPluginsPerHost   = 256;
constexpr int        kPPQ                 = 96;  // pulses per quarter note

// ---------------------------------------------------------------------------
// Note range
// ---------------------------------------------------------------------------
constexpr NoteNumber kNoteMin = 0;
constexpr NoteNumber kNoteMax = 127;
constexpr NoteNumber kMiddleC = 60;

// ---------------------------------------------------------------------------
// Gain / pan helpers
// ---------------------------------------------------------------------------
struct GainPan {
    float gain = 1.0f;  // linear, [0, 4]
    float pan  = 0.0f;  // [-1, +1], 0 = centre

    float leftGain()  const noexcept { return gain * (pan <= 0.0f ? 1.0f : 1.0f - pan); }
    float rightGain() const noexcept { return gain * (pan >= 0.0f ? 1.0f : 1.0f + pan); }
};

// ---------------------------------------------------------------------------
// Plugin format
// ---------------------------------------------------------------------------
enum class PluginFormat : uint8_t {
    Unknown = 0,
    VST2,
    VST3,
    AU,     // macOS only
    Internal
};

inline const char* pluginFormatName(PluginFormat f) noexcept {
    switch (f) {
        case PluginFormat::VST2:     return "VST2";
        case PluginFormat::VST3:     return "VST3";
        case PluginFormat::AU:       return "AU";
        case PluginFormat::Internal: return "Internal";
        default:                     return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// Theme colour (ARGB packed)
// ---------------------------------------------------------------------------
struct Color {
    uint8_t a = 255, r = 0, g = 0, b = 0;

    constexpr uint32_t packed() const noexcept {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) <<  8) |
               static_cast<uint32_t>(b);
    }

    static constexpr Color fromPacked(uint32_t p) noexcept {
        return { static_cast<uint8_t>(p >> 24),
                 static_cast<uint8_t>(p >> 16),
                 static_cast<uint8_t>(p >>  8),
                 static_cast<uint8_t>(p) };
    }

    static constexpr Color Black()  noexcept { return {255,   0,   0,   0}; }
    static constexpr Color White()  noexcept { return {255, 255, 255, 255}; }
    static constexpr Color Red()    noexcept { return {255, 255,   0,   0}; }
    static constexpr Color Green()  noexcept { return {255,   0, 255,   0}; }
    static constexpr Color Blue()   noexcept { return {255,   0,   0, 255}; }
    static constexpr Color Transparent() noexcept { return {0, 0, 0, 0}; }
};

// ---------------------------------------------------------------------------
// Audio buffer (non-owning view over interleaved float samples)
// ---------------------------------------------------------------------------
struct AudioBuffer {
    float*       data     = nullptr;
    FrameCount   frames   = 0;
    uint32_t     channels = 2;

    float& sample(uint32_t channel, FrameCount frame) noexcept {
        return data[frame * channels + channel];
    }
    const float& sample(uint32_t channel, FrameCount frame) const noexcept {
        return data[frame * channels + channel];
    }
    void zero() noexcept {
        if (data) std::fill(data, data + static_cast<size_t>(frames) * channels, 0.0f);
    }
};

// ---------------------------------------------------------------------------
// Callback types
// ---------------------------------------------------------------------------
using AudioCallback  = std::function<void(AudioBuffer& output, FrameCount frames)>;
using MidiCallback   = std::function<void(NoteNumber note, Velocity vel, bool noteOn)>;
using ErrorCallback  = std::function<void(const std::string& message)>;

} // namespace FL
