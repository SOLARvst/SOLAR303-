#include "SOLAR303Sequencer.h"
#include <cmath>
#include <cstring>

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void SOLAR303Sequencer::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
}

void SOLAR303Sequencer::reset()
{
    samplePos  = 0.0;
    lastStep   = -1;
    noteActive = false;
    currentStep.store(-1);
}

void SOLAR303Sequencer::stop()
{
    // Only set atomic flags — samplePos/lastStep are reset by the audio thread
    playing.store(false);
    pendingReset.store(true);
    currentStep.store(-1);
}

// ── Audio thread ──────────────────────────────────────────────────────────────

double SOLAR303Sequencer::samplesPerStep() const
{
    // 16th notes: one beat = 4 steps
    return sr * 60.0 / (double(bpm.load()) * 4.0);
}

void SOLAR303Sequencer::process(SOLAR303Engine& engine, int numSamples)
{
    if (!playing.load()) return;

    // Safely reset position on the audio thread (avoids data race on samplePos/lastStep)
    if (pendingReset.load())
    {
        samplePos  = 0.0;
        lastStep   = -1;
        noteActive = false;
        pendingReset.store(false);
    }

    const double spStep = samplesPerStep();

    for (int i = 0; i < numSamples; ++i)
    {
        const int len = std::max(1, std::min(NUM_STEPS, numSteps.load()));
        const int newStep = (int)(samplePos / spStep) % len;

        if (newStep != lastStep)
        {
            const int  prevActiveNote = activeNote;
            const bool prevSlide = (lastStep >= 0) && steps[lastStep].slide;

            lastStep = newStep;
            currentStep.store(newStep);

            const SequencerStep& s = steps[newStep];

            if (prevSlide && noteActive)
            {
                // ── Slide transition ─────────────────────────────────────────
                // Fire new note FIRST so the engine detects a non-empty
                // heldNotes list and enters legato/slide mode.
                // Then release the old note so heldNotes doesn't accumulate.
                if (s.active)
                {
                    engine.noteOn(s.note, s.accent ? 1.0f : 0.55f);
                    activeNote = s.note;
                    noteActive = true;
                }
                engine.noteOff(prevActiveNote);   // removes old note, keeps new
            }
            else
            {
                // ── Normal transition ─────────────────────────────────────────
                // Clear all held notes first to guarantee a fresh envelope attack.
                if (noteActive)
                {
                    engine.allNotesOff();
                    noteActive = false;
                }
                if (s.active)
                {
                    engine.noteOn(s.note, s.accent ? 1.0f : 0.55f);
                    activeNote = s.note;
                    noteActive = true;
                }
            }
        }

        samplePos += 1.0;

        // Wrap samplePos to one cycle to prevent floating-point precision loss
        const double cycleLen = spStep * len;
        if (samplePos >= cycleLen * 2.0)
            samplePos -= cycleLen;
    }
}

// ── Utilities ─────────────────────────────────────────────────────────────────

std::string SOLAR303Sequencer::noteToString(int note)
{
    static const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    const int octave = note / 12 - 1;
    return std::string(names[note % 12]) + std::to_string(octave);
}
