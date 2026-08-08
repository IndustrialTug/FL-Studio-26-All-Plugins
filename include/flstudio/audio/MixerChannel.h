#pragma once
#include "flstudio/audio/AudioEngine.h"
#include <cmath>

namespace fl {

static inline float dbToLinear(float db) { return std::pow(10.0f, db / 20.0f); }

void applyEQ(MixerChannel& ch, float* buf, int n);
float applyCompressor(MixerChannel& ch, float sample);

} // namespace fl