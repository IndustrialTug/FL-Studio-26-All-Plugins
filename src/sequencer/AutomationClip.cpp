#include "flstudio/sequencer/AutomationClip.h"
#include <algorithm>
#include <cmath>

namespace fl {

static float lerp(float a, float b, float t) { return a + (b - a) * t; }

static float bezierEase(float t, float tension)
{
    float e = std::clamp(tension, -1.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t) * (1.0f + e * (t - 0.5f));
}

void AutomationClip::addBreakpoint(int tick, float value, float tension)
{
    Breakpoint bp{ tick, value, tension };
    m_points.push_back(bp);
    std::sort(m_points.begin(), m_points.end(),
              [](const Breakpoint& a, const Breakpoint& b){ return a.tick < b.tick; });
}

float AutomationClip::evaluate(int tick) const
{
    if (m_points.empty()) return 0.0f;
    if (tick <= m_points.front().tick) return m_points.front().value;
    if (tick >= m_points.back().tick)  return m_points.back().value;
    for (size_t i = 1; i < m_points.size(); ++i) {
        const auto& a = m_points[i - 1];
        const auto& b = m_points[i];
        if (tick >= a.tick && tick < b.tick) {
            float t = (float)(tick - a.tick) / (b.tick - a.tick);
            t = bezierEase(t, a.tension);
            return lerp(a.value, b.value, t);
        }
    }
    return 0.0f;
}

void AutomationClip::clear() { m_points.clear(); }

} // namespace fl