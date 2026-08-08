#include "PatternSequencer.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace FL {

// ---------------------------------------------------------------------------
// Pattern helpers
// ---------------------------------------------------------------------------

void Pattern::buildFromStepGrid()
{
    if (!stepGrid) return;

    notes.clear();
    const Tick ticksPerStep = kPPQ / std::max(1u, stepGrid->stepsPerBeat);

    for (size_t i = 0; i < stepGrid->size(); ++i)
    {
        const auto& step = (*stepGrid)[i];
        if (!step.active) continue;

        MidiNote n;
        n.startTick  = static_cast<Tick>(i) * ticksPerStep;
        n.endTick    = n.startTick + ticksPerStep;
        n.note       = step.note;
        n.velocity   = step.velocity;
        n.channel    = 0;
        notes.push_back(n);
    }
}

// ---------------------------------------------------------------------------
// PatternSequencer — pattern management
// ---------------------------------------------------------------------------

PatternId PatternSequencer::createPattern(std::string name)
{
    const PatternId id = nextId();
    Pattern p;
    p.id   = id;
    p.name = name.empty() ? ("Pattern " + std::to_string(id)) : std::move(name);
    m_patterns.emplace_back(id, std::move(p));

    if (m_selectedPattern == 0)
        m_selectedPattern = id;

    return id;
}

void PatternSequencer::deletePattern(PatternId id)
{
    m_patterns.erase(
        std::remove_if(m_patterns.begin(), m_patterns.end(),
            [id](const auto& pair) { return pair.first == id; }),
        m_patterns.end());

    if (m_selectedPattern == id)
        m_selectedPattern = m_patterns.empty() ? 0 : m_patterns.front().first;
}

Pattern* PatternSequencer::getPattern(PatternId id)
{
    for (auto& [pid, pat] : m_patterns)
        if (pid == id) return &pat;
    return nullptr;
}

const Pattern* PatternSequencer::getPattern(PatternId id) const
{
    for (const auto& [pid, pat] : m_patterns)
        if (pid == id) return &pat;
    return nullptr;
}

void PatternSequencer::selectPattern(PatternId id)
{
    m_selectedPattern = id;
}

// ---------------------------------------------------------------------------
// Transport
// ---------------------------------------------------------------------------

void PatternSequencer::play() noexcept  { m_playing = true; }
void PatternSequencer::pause() noexcept { m_playing = false; }
void PatternSequencer::stop() noexcept  { m_playing = false; m_currentTick = 0; m_tickAccum = 0.0; }

void PatternSequencer::setBpm(float bpm) noexcept
{
    m_bpm = std::max(1.0f, bpm);
}

// ---------------------------------------------------------------------------
// Per-buffer advance
// ---------------------------------------------------------------------------

void PatternSequencer::advance(FrameCount frames, SampleRate sampleRate)
{
    if (!m_playing || m_selectedPattern == 0 || sampleRate == 0 || frames == 0)
        return;

    const Pattern* pat = getPattern(m_selectedPattern);
    if (!pat || pat->notes.empty() || pat->length <= 0)
        return;

    // How many ticks does this buffer span?
    // ticksPerSample = (BPM / 60) * PPQ / sampleRate
    const double ticksPerSample = (m_bpm / 60.0) * static_cast<double>(kPPQ) / static_cast<double>(sampleRate);
    const double ticksThisBuffer = ticksPerSample * static_cast<double>(frames);

    const Tick bufferStartTick = m_currentTick;
    const Tick bufferEndTick   = bufferStartTick + static_cast<Tick>(std::ceil(ticksThisBuffer));

    // Scan all notes in the pattern and fire callbacks for events in this window.
    // Handle pattern loop wrapping.
    const Tick patLen = pat->length;

    for (const auto& note : pat->notes)
    {
        // Note-on
        Tick noteStart = note.startTick;
        // Wrap into current loop cycle
        Tick loopOffset = (bufferStartTick / patLen) * patLen;
        Tick absoluteStart = noteStart + loopOffset;

        if (absoluteStart >= bufferStartTick && absoluteStart < bufferEndTick)
        {
            if (m_noteOnCb) m_noteOnCb(note);
        }

        // Note-off
        Tick absoluteEnd = note.endTick + loopOffset;
        if (absoluteEnd >= bufferStartTick && absoluteEnd < bufferEndTick)
        {
            if (m_noteOffCb) m_noteOffCb(note);
        }
    }

    // Advance position, wrapping at pattern end
    m_tickAccum += ticksThisBuffer;
    const Tick advance_ticks = static_cast<Tick>(m_tickAccum);
    m_tickAccum -= static_cast<double>(advance_ticks);

    m_currentTick = (m_currentTick + advance_ticks) % patLen;
}

} // namespace FL
