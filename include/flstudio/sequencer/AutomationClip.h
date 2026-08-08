#pragma once
#include <vector>
#include <algorithm>

namespace fl {

struct Breakpoint {
    int   tick     = 0;
    float value    = 0.0f;
    float tension  = 0.0f;
};

class AutomationClip {
public:
    void  addBreakpoint(int tick, float value, float tension = 0.0f);
    float evaluate(int tick) const;
    void  clear();
    const std::vector<Breakpoint>& points() const { return m_points; }

private:
    std::vector<Breakpoint> m_points;
};

} // namespace fl