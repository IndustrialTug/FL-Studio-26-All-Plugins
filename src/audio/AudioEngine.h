#pragma once
#include "../../include/FLTypes.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>

namespace FL {

// ---------------------------------------------------------------------------
// AudioEngine
// Manages the audio I/O loop: opens a device, drives a render callback at the
// requested sample rate and buffer size, and exposes start/stop control.
// Thread-safe: start/stop may be called from any thread.
// ---------------------------------------------------------------------------
class AudioEngine {
public:
    struct Config {
        SampleRate   sampleRate    = kDefaultSampleRate;
        FrameCount   bufferFrames  = kDefaultBufferFrames;
        uint32_t     outputChannels = 2;
        std::string  deviceName;   // empty = system default
    };

    explicit AudioEngine(const Config& cfg = {});
    ~AudioEngine();

    // Non-copyable
    AudioEngine(const AudioEngine&)            = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // Start the audio processing loop.  Returns false if already running or
    // if the device could not be opened.
    bool start();

    // Stop the audio processing loop and block until the render thread exits.
    void stop();

    bool isRunning() const noexcept { return m_running.load(std::memory_order_acquire); }

    // Register the callback that will be invoked on the audio thread to fill
    // each output buffer.  Must be called before start(), or the engine
    // produces silence.
    void setAudioCallback(AudioCallback cb);

    // Register a callback invoked on the audio thread when an error occurs
    // (e.g. device disconnect).
    void setErrorCallback(ErrorCallback cb);

    SampleRate  getSampleRate()   const noexcept { return m_config.sampleRate; }
    FrameCount  getBufferFrames() const noexcept { return m_config.bufferFrames; }
    uint32_t    getChannelCount() const noexcept { return m_config.outputChannels; }

    // Cumulative frames rendered since start() was last called.
    uint64_t framesRendered() const noexcept { return m_framesRendered.load(std::memory_order_relaxed); }

    // CPU load estimate: fraction of buffer period spent in the render callback.
    // Range [0, 1].  Updated once per buffer.
    float cpuLoad() const noexcept { return m_cpuLoad.load(std::memory_order_relaxed); }

private:
    void renderLoop();
    void simulateDeviceOpen();   // platform-agnostic stub; replace with WASAPI/CoreAudio

    Config               m_config;
    AudioCallback        m_audioCallback;
    ErrorCallback        m_errorCallback;
    std::mutex           m_callbackMutex;

    std::thread          m_renderThread;
    std::atomic<bool>    m_running{false};
    std::atomic<bool>    m_stopRequested{false};

    std::vector<float>   m_outputBuffer;   // interleaved, frames * channels
    std::atomic<uint64_t> m_framesRendered{0};
    std::atomic<float>   m_cpuLoad{0.0f};
};

} // namespace FL
