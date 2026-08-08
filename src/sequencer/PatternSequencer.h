#pragma once
#include "MidiNote.h"
#include "../../include/FLTypes.h"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace FL {

// ---------------------------------------------------------------------------
// StepGrid
// A fixed-length step-sequencer grid for one voice / instrument track.
// Each step can be on or off, with optional per-step velocity and pitch.
// ---------------------------------------------------------------------------
struct StepGrid {
    static constexpr size_t kMaxSteps = 64;

    struct Step {
        bool      active   = false;
        NoteNumber note     = kMiddleC;
        Velocity   velocity = 100;
    };

    std::vector<Step> steps;
    uint32_t          stepsPerBeat = 4;  // 4 = 16th-note resolution at 4/4

    explicit StepGrid(size_t numSteps = 16, uint32_t stepsPerBeat = 4)
        : steps(numSteps), stepsPerBeat(stepsPerBeat)
    {}

    size_t   size()                    const noexcept { return steps.size(); }
    Step&    operator[](size_t i)            noexcept { return steps[i]; }
    const Step& operator[](size_t i)   const noexcept { return steps[i]; }

    void resize(size_t n) { steps.resize(n); }
    void clear()          { for (auto& s : steps) s = {}; }
};

// ---------------------------------------------------------------------------
// Pattern
// A named collection of MidiNotes with a defined length (in ticks).
// ---------------------------------------------------------------------------
struct Pattern {
    PatternId            id     = 0;
    std::string          name;
    Tick                 length = kPPQ * 4;   // default: 1 bar at 4/4
    std::vector<MidiNote> notes;
    std::optional<StepGrid> stepGrid;         // optional step-sequencer view

    void addNote(const MidiNote& n)   { notes.push_back(n); }
    void removeNote(size_t index)     { notes.erase(notes.begin() + index); }
    void sortNotes()                  { std::sort(notes.begin(), notes.end()); }
    void clearNotes()                 { notes.clear(); }

    // Build MIDI notes from the step grid (overwrites existing notes).
    void buildFromStepGrid();
};

// ---------------------------------------------------------------------------
// PatternSequencer
// Manages a list of Patterns and drives playback: tracks the current play
// position, advances it per-buffer, and fires note callbacks at the right
// moment.
// ---------------------------------------------------------------------------
class PatternSequencer {
public:
    PatternSequencer() = default;
    ~PatternSequencer() = default;

    // --- Pattern management ---
    PatternId createPattern(std::string name = "");
    void      deletePattern(PatternId id);
    Pattern*  getPattern(PatternId id);
    const Pattern* getPattern(PatternId id) const;

    size_t patternCount() const noexcept { return m_patterns.size(); }

    // Set the pattern to play.  Does not reset playback position.
    void selectPattern(PatternId id);
    PatternId selectedPatternId() const noexcept { return m_selectedPattern; }

    // --- Transport ---
    void   play()  noexcept;
    void   pause() noexcept;
    void   stop()  noexcept;  // also rewinds to tick 0
    bool   isPlaying() const noexcept { return m_playing; }

    Tick   currentTick() const noexcept { return m_currentTick; }
    void   seekTo(Tick tick) noexcept   { m_currentTick = tick; }

    // Tempo in BPM.
    float  getBpm() const noexcept { return m_bpm; }
    void   setBpm(float bpm) noexcept;

    // --- Per-buffer advance ---
    // Call once per audio buffer.  Fires m_noteOnCb / m_noteOffCb for every
    // note that starts or ends within the buffer window.
    void advance(FrameCount frames, SampleRate sampleRate);

    // --- Callbacks ---
    void setNoteOnCallback(std::function<void(const MidiNote&)> cb)  { m_noteOnCb  = std::move(cb); }
    void setNoteOffCallback(std::function<void(const MidiNote&)> cb) { m_noteOffCb = std::move(cb); }

private:
    PatternId nextId() noexcept { return ++m_idCounter; }

    std::vector<std::pair<PatternId, Pattern>> m_patterns;
    PatternId   m_selectedPattern = 0;
    PatternId   m_idCounter       = 0;

    bool        m_playing     = false;
    Tick        m_currentTick = 0;
    float       m_bpm         = 140.0f;

    // Fractional tick accumulator (sub-tick precision across buffers)
    double      m_tickAccum  = 0.0;

    std::function<void(const MidiNote&)> m_noteOnCb;
    std::function<void(const MidiNote&)> m_noteOffCb;
};

} // namespace FL
