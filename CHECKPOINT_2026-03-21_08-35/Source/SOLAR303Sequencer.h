#pragma once
#include "SOLAR303Engine.h"
#include <atomic>
#include <string>

// ── One sequencer step ────────────────────────────────────────────────────────
struct SequencerStep
{
    int  note   = 48;    // MIDI note (48 = C3)
    bool active = true;  // step on/off
    bool accent = false; // accent (high velocity + filter boost)
    bool slide  = false; // slide/portamento to next note
};

// ── TB-303 Step Sequencer ─────────────────────────────────────────────────────
class SOLAR303Sequencer
{
public:
    static constexpr int NUM_STEPS = 16;

    // Step data — read/written from UI thread, read from audio thread
    SequencerStep steps[NUM_STEPS];

    std::atomic<float> bpm          { 120.f };
    std::atomic<bool>  playing      { false };
    std::atomic<bool>  pendingReset { false };  // UI → audio: reset on next process()
    std::atomic<int>   currentStep  { -1 };
    std::atomic<int>   numSteps     { 16 };     // 1–16: active step count

    double sr = 44100.0;  // written only from audio thread (prepareToPlay/process)

    void prepare(double sampleRate);
    void reset();
    void stop();

    // Called per-sample from processBlock on the audio thread
    void process(SOLAR303Engine& engine, int numSamples);

    // Utility
    static std::string noteToString(int midiNote);
    static int         noteFromString(const std::string& s);

private:
    double samplePos   = 0.0;
    int    lastStep    = -1;
    bool   noteActive  = false;
    int    activeNote  = 48;

    double samplesPerStep() const;
};
