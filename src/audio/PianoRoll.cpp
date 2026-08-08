#include "flstudio/audio/PianoRoll.h"

namespace fl {

void PianoRoll::addNote(int pitch, int startTick, int lengthTicks, int velocity)
{
    Note n;
    n.pitch      = std::clamp(pitch, 0, 127);
    n.startTick  = snapToGrid(startTick);
    n.lengthTicks = std::max(lengthTicks, m_gridSize);
    n.velocity   = std::clamp(velocity, 1, 127);
    m_notes.push_back(n);
    std::sort(m_notes.begin(), m_notes.end(),
              [](const Note& a, const Note& b){ return a.startTick < b.startTick; });
}

void PianoRoll::removeNote(int index)
{
    if (index >= 0 && index < (int)m_notes.size())
        m_notes.erase(m_notes.begin() + index);
}

void PianoRoll::quantize(int gridSize)
{
    m_gridSize = gridSize;
    for (auto& n : m_notes)
        n.startTick = snapToGrid(n.startTick);
}

std::vector<Note> PianoRoll::getNotesInRange(int startTick, int endTick) const
{
    std::vector<Note> result;
    for (auto& n : m_notes)
        if (n.startTick >= startTick && n.startTick < endTick)
            result.push_back(n);
    return result;
}

} // namespace fl