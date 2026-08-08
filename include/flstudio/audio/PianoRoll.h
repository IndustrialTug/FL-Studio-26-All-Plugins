#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace fl {

struct Note {
    int pitch       = 60;
    int startTick   = 0;
    int lengthTicks = 96;
    int velocity    = 100;
};

class PianoRoll {
public:
    explicit PianoRoll(int gridSize = 16) : m_gridSize(gridSize) {}

    void addNote(int pitch, int startTick, int lengthTicks, int velocity);
    void removeNote(int index);
    void quantize(int gridSize);
    std::vector<Note> getNotesInRange(int startTick, int endTick) const;
    const std::vector<Note>& notes() const { return m_notes; }
    int gridSize() const { return m_gridSize; }

private:
    int  m_gridSize;
    std::vector<Note> m_notes;

    int snapToGrid(int tick) const { return (tick / m_gridSize) * m_gridSize; }
};

} // namespace fl