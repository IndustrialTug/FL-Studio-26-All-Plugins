#include "AudioEngine.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <cstring>

namespace FL {

AudioEngine::AudioEngine(const Config& cfg)
    : m_config(cfg)
{
    // Pre-allocate output buffer (interleaved stereo)
    m_outputBuffer.resize(static_cast<size_t>(cfg.bufferFrames) * cfg.outputChannels, 0.0f);
}

AudioEngine::~AudioEngine()
{
    stop();
}

void AudioEngine::setAudioCallback(AudioCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_audioCallback = std::move(cb);
}

void AudioEngine::setErrorCallback(ErrorCallback cb)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_errorCallback = std::move(cb);
}

bool AudioEngine::start()
{
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
        return false;  // already running

    m_stopRequested.store(false, std::memory_order_release);
    m_framesRendered.store(0, std::memory_order_relaxed);

    try {
        simulateDeviceOpen();
    } catch (const std::exception& e) {
        m_running.store(false, std::memory_order_release);
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_errorCallback)
            m_errorCallback(e.what());
        return false;
    }

    m_renderThread = std::thread(&AudioEngine::renderLoop, this);
    return true;
}

void AudioEngine::stop()
{
    m_stopRequested.store(true, std::memory_order_release);
    if (m_renderThread.joinable())
        m_renderThread.join();
    m_running.store(false, std::memory_order_release);
}

void AudioEngine::simulateDeviceOpen()
{
    // In a real implementation this would open a WASAPI (Windows) or
    // CoreAudio (macOS) device at the requested sample rate and buffer size.
    // For the platform-agnostic simulation we just validate parameters.
    if (m_config.sampleRate < 8000 || m_config.sampleRate > 192000)
        throw std::runtime_error("AudioEngine: invalid sample rate");
    if (m_config.bufferFrames == 0)
        throw std::runtime_error("AudioEngine: buffer size cannot be zero");
}

void AudioEngine::renderLoop()
{
    // Duration of one buffer period in microseconds.
    const double periodUs = (static_cast<double>(m_config.bufferFrames) / m_config.sampleRate) * 1e6;
    const auto   period   = std::chrono::microseconds(static_cast<int64_t>(periodUs));

    AudioBuffer buf;
    buf.data     = m_outputBuffer.data();
    buf.frames   = m_config.bufferFrames;
    buf.channels = m_config.outputChannels;

    while (!m_stopRequested.load(std::memory_order_acquire))
    {
        const auto callStart = std::chrono::high_resolution_clock::now();

        // Zero output buffer then invoke render callback
        buf.zero();
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            if (m_audioCallback)
                m_audioCallback(buf, m_config.bufferFrames);
        }

        const auto callEnd = std::chrono::high_resolution_clock::now();
        const double elapsedUs = std::chrono::duration<double, std::micro>(callEnd - callStart).count();
        m_cpuLoad.store(static_cast<float>(elapsedUs / periodUs), std::memory_order_relaxed);

        m_framesRendered.fetch_add(m_config.bufferFrames, std::memory_order_relaxed);

        // Simulate hardware timing: sleep for the remainder of the buffer period.
        const auto remaining = period - std::chrono::duration_cast<std::chrono::microseconds>(callEnd - callStart);
        if (remaining > std::chrono::microseconds(0))
            std::this_thread::sleep_for(remaining);
    }
}

} // namespace FL
