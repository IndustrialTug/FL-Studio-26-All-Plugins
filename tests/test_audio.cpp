// test_audio.cpp
// Unit tests for AudioEngine and MixerChannel
// Build: CMake target "tests" or compile with: g++ -std=c++17 test_audio.cpp ../src/audio/AudioEngine.cpp ../src/audio/MixerChannel.cpp -I../include -lpthread -o test_audio

#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>
#include "../src/audio/AudioEngine.h"
#include "../src/audio/MixerChannel.h"

using namespace FL;

// ---------------------------------------------------------------------------
// Tiny assertion helper
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                   \
    do {                                                                   \
        if (cond) { ++g_pass; }                                            \
        else {                                                             \
            ++g_fail;                                                      \
            std::cerr << "[FAIL] " << msg << "  (" #cond ")\n";           \
        }                                                                  \
    } while (false)

#define CHECK_NEAR(a, b, eps, msg)                                         \
    CHECK(std::fabs((a) - (b)) <= (eps), msg)

// ---------------------------------------------------------------------------
// MixerChannel tests
// ---------------------------------------------------------------------------

static void test_mixer_channel_default_state()
{
    MixerChannel ch(0, "Master");
    CHECK(ch.getIndex() == 0,         "channel index");
    CHECK(ch.getName()  == "Master",  "channel name");
    CHECK_NEAR(ch.getVolume(), 1.0f, 1e-6f, "default volume");
    CHECK_NEAR(ch.getPan(),    0.0f, 1e-6f, "default pan");
    CHECK(!ch.isMuted(),  "default not muted");
    CHECK(!ch.isSoloed(), "default not soloed");
}

static void test_mixer_channel_volume_clamp()
{
    MixerChannel ch(1);
    ch.setVolume(-1.0f);
    CHECK_NEAR(ch.getVolume(), 0.0f, 1e-6f, "volume clamp low");
    ch.setVolume(10.0f);
    CHECK_NEAR(ch.getVolume(), 4.0f, 1e-6f, "volume clamp high");
    ch.setVolume(0.75f);
    CHECK_NEAR(ch.getVolume(), 0.75f, 1e-6f, "volume in range");
}

static void test_mixer_channel_pan_clamp()
{
    MixerChannel ch(2);
    ch.setPan(-5.0f);
    CHECK_NEAR(ch.getPan(), -1.0f, 1e-6f, "pan clamp low");
    ch.setPan(5.0f);
    CHECK_NEAR(ch.getPan(),  1.0f, 1e-6f, "pan clamp high");
    ch.setPan(0.5f);
    CHECK_NEAR(ch.getPan(),  0.5f, 1e-6f, "pan in range");
}

static void test_mixer_channel_mute_silences_output()
{
    MixerChannel ch(3);
    ch.setMuted(true);

    std::vector<float> buf(8, 1.0f);   // 4 frames, stereo, all 1.0
    ch.process(buf.data(), 4);

    for (float s : buf)
        CHECK_NEAR(s, 0.0f, 1e-6f, "muted channel output = 0");
}

static void test_mixer_channel_process_unity()
{
    MixerChannel ch(4);
    // volume=1, pan=0 => both channels gain=1
    std::vector<float> buf(8);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<float>(i + 1);

    std::vector<float> orig = buf;
    ch.process(buf.data(), 4);

    for (size_t i = 0; i < buf.size(); ++i)
        CHECK_NEAR(buf[i], orig[i], 1e-5f, "unity gain passthrough");
}

static void test_mixer_channel_sends()
{
    MixerChannel ch(5);
    CHECK(!ch.hasSend(10), "no sends initially");

    ch.addSend(10, 1.0f);
    CHECK(ch.hasSend(10), "send added");
    CHECK(ch.getSends().size() == 1, "one send");

    ch.addSend(10, 0.5f);   // update existing
    CHECK(ch.getSends().size() == 1, "no duplicate send");
    CHECK_NEAR(ch.getSends()[0].amount, 0.5f, 1e-6f, "send amount updated");

    ch.removeSend(10);
    CHECK(!ch.hasSend(10), "send removed");
}

// ---------------------------------------------------------------------------
// AudioEngine tests
// ---------------------------------------------------------------------------

static void test_audio_engine_start_stop()
{
    AudioEngine::Config cfg;
    cfg.sampleRate   = 44100;
    cfg.bufferFrames = 256;

    AudioEngine engine(cfg);
    CHECK(!engine.isRunning(), "not running before start");

    const bool ok = engine.start();
    CHECK(ok, "start() returns true");
    CHECK(engine.isRunning(), "running after start");

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    engine.stop();
    CHECK(!engine.isRunning(), "not running after stop");
}

static void test_audio_engine_double_start()
{
    AudioEngine engine;
    engine.start();
    const bool second = engine.start();
    CHECK(!second, "second start() returns false");
    engine.stop();
}

static void test_audio_engine_frames_rendered()
{
    AudioEngine::Config cfg;
    cfg.sampleRate   = 44100;
    cfg.bufferFrames = 128;

    AudioEngine engine(cfg);
    engine.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    engine.stop();

    // At 44100 Hz, 128-frame buffers, 50 ms → at least ~17 buffers
    CHECK(engine.framesRendered() > 0, "frames rendered > 0");
}

static void test_audio_engine_callback_invoked()
{
    AudioEngine::Config cfg;
    cfg.sampleRate   = 44100;
    cfg.bufferFrames = 512;

    AudioEngine engine(cfg);
    std::atomic<int> callCount{0};
    engine.setAudioCallback([&](AudioBuffer& buf, FrameCount frames) {
        ++callCount;
        // Fill buffer with a constant so we can verify it later
        for (FrameCount i = 0; i < frames * 2; ++i)
            buf.data[i] = 0.5f;
    });

    engine.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    engine.stop();

    CHECK(callCount.load() > 0, "audio callback was invoked");
}

static void test_audio_engine_cpu_load_range()
{
    AudioEngine engine;
    engine.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const float load = engine.cpuLoad();
    engine.stop();

    CHECK(load >= 0.0f && load <= 1.0f, "cpuLoad in [0,1]");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    std::cout << "=== AudioEngine / MixerChannel tests ===\n";

    test_mixer_channel_default_state();
    test_mixer_channel_volume_clamp();
    test_mixer_channel_pan_clamp();
    test_mixer_channel_mute_silences_output();
    test_mixer_channel_process_unity();
    test_mixer_channel_sends();

    test_audio_engine_start_stop();
    test_audio_engine_double_start();
    test_audio_engine_frames_rendered();
    test_audio_engine_callback_invoked();
    test_audio_engine_cpu_load_range();

    std::cout << "Passed: " << g_pass << "  Failed: " << g_fail << "\n";
    return (g_fail == 0) ? 0 : 1;
}
