#pragma once
#include "../../include/FLTypes.h"
#include <string>
#include <vector>
#include <optional>

namespace FL {

// ---------------------------------------------------------------------------
// MidiNote
// Represents a single MIDI note event in a pattern or sequence.
// ---------------------------------------------------------------------------
struct MidiNote {
    Tick       startTick  = 0;     // note-on position (ticks from pattern start)
    Tick       endTick    = 0;     // note-off position
    NoteNumber note       = 60;    // MIDI pitch (0-127, 60 = middle C)
    Velocity   velocity   = 100;   // MIDI velocity (0-127)
    uint8_t    channel    = 0;     // MIDI channel (0-15)
    float      pan        = 0.0f;  // per-note pan override [-1, +1]; NaN = use channel
    float      pitchBend  = 0.0f;  // semitones, [-12, +12]

    // Duration in ticks
    Tick duration() const noexcept { return endTick - startTick; }

    // Duration in beats (quarter notes) given a PPQ value
    float durationBeats(int ppq = kPPQ) const noexcept {
        return static_cast<float>(duration()) / static_cast<float>(ppq);
    }

    // Start time in beats
    float startBeats(int ppq = kPPQ) const noexcept {
        return static_cast<float>(startTick) / static_cast<float>(ppq);
    }

    // Comparator: sort by start tick, then note number
    bool operator<(const MidiNote& o) const noexcept {
        if (startTick != o.startTick) return startTick < o.startTick;
        return note < o.note;
    }

    bool operator==(const MidiNote& o) const noexcept {
        return startTick == o.startTick && endTick == o.endTick &&
               note == o.note && velocity == o.velocity && channel == o.channel;
    }
};

// ---------------------------------------------------------------------------
// Helper factories
// ---------------------------------------------------------------------------

// Create a note at `startTick` with a duration expressed in ticks.
inline MidiNote makeNote(NoteNumber n, Velocity vel, Tick startTick, Tick durationTicks,
                         uint8_t channel = 0)
{
    MidiNote m;
    m.note       = n;
    m.velocity   = vel;
    m.startTick  = startTick;
    m.endTick    = startTick + durationTicks;
    m.channel    = channel;
    return m;
}

// Create a note with duration expressed in beats (quarter notes).
inline MidiNote makeNoteBeats(NoteNumber n, Velocity vel, float startBeat,
                               float durationBeats, uint8_t channel = 0, int ppq = kPPQ)
{
    return makeNote(n, vel,
        static_cast<Tick>(startBeat    * ppq),
        static_cast<Tick>(durationBeats * ppq),
        channel);
}

// Scale a note's velocity by a multiplier in [0, 2].
inline MidiNote scaleVelocity(MidiNote note, float factor) noexcept
{
    const int v = static_cast<int>(note.velocity * factor + 0.5f);
    note.velocity = static_cast<Velocity>(std::clamp(v, 0, 127));
    return note;
}

// Transpose a note by `semitones`, clamping to [kNoteMin, kNoteMax].
inline MidiNote transpose(MidiNote note, int semitones) noexcept
{
    const int n = static_cast<int>(note.note) + semitones;
    note.note = static_cast<NoteNumber>(std::clamp(n, (int)kNoteMin, (int)kNoteMax));
    return note;
}

// Shift a note's position by `ticks` (can be negative).
inline MidiNote shift(MidiNote note, Tick ticks) noexcept
{
    note.startTick += ticks;
    note.endTick   += ticks;
    return note;
}

} // namespace FL
