// AutomationClip.h — breakpoint curve engine header for FL Studio toolkit

#pragma once
#include <vector>

struct Breakpoint { int tick; float value; float tension = 0.0f; };

class AutomationClip {
public:
    void  addBreakpoint(int tick, float value, float tension = 0.0f);
    float evaluate(int tick) const;
    void  clear();
    const std::vector<Breakpoint>& getPoints() const { return m_points; }
private:
    std::vector<Breakpoint> m_points;
};
