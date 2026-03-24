#pragma once

// ── SOLAR 303 Factory Presets ─────────────────────────────────────────────────
// Embedded in the plugin — no files, no folders needed.
// Cubase shows these in the preset selector at the top of the plugin window.

struct FactoryPresetData
{
    const char* name;

    // ── Synth (actual values, not normalised) ────────────────────────────────
    float cutoff;       // 60–5000 Hz
    float resonance;    // 0–1
    float envmod;       // 0–1
    float decay;        // 0.2–2.5 s
    float accent;       // 0–1
    float slide;        // 0.02–0.3 s
    int   waveform;     // 0=Saw  1=Square
    float volume;       // 0–1
    float mastervol;    // 0–1
    float tuning;       // -24 … +24 semitones

    // ── FX ──────────────────────────────────────────────────────────────────
    bool  revOn;
    float revSize, revDamp, revMix;

    bool  delayOn;
    float delayTime, delayFeed, delayMix;

    bool  distOn;
    int   distType;     // 0=Tanh 1=Hard 2=Fuzz 3=Fold 4=Tube 5=Crush
    float distAmount, distVol;

    // ── Sequencer ────────────────────────────────────────────────────────────
    float bpm;
    int   numSteps;

    struct Step { int note; bool active, accent, slide; } steps[16];
};

// ── NOTE REFERENCE (Cubase convention: C3 = MIDI 60) ─────────────────────────
// C1=36  D1=38  E1=40  F1=41  G1=43  A1=45  B1=47
// C2=48  D2=50  E2=52  F2=53  G2=55  A2=57  B2=59
// C3=60  D3=62  E3=64  F3=65  G3=67  A3=69

static const FactoryPresetData FACTORY_PRESETS[] =
{
    // ── 0: Acid Classic ──────────────────────────────────────────────────────
    {
        "Acid Classic",
        345.f, 0.95f, 0.29f, 0.50f, 0.16f, 0.06f, 0, 1.f, 1.f, 0.f,
        false, 0.42f, 0.50f, 0.15f,
        false, 0.375f, 0.40f, 0.30f,
        false, 0, 0.5f, 0.7f,
        130.f, 16,
        {
            {45,true, false,false}, {45,false,false,false},
            {45,true, false,false}, {48,true, true, false},
            {48,false,false,false}, {45,true, false,false},
            {43,true, false,true }, {45,true, false,false},
            {45,true, false,false}, {45,false,false,false},
            {50,true, false,false}, {50,true, true, false},
            {48,false,false,false}, {45,true, false,false},
            {43,true, false,false}, {48,true, false,false},
        }
    },

    // ── 1: Squelch ───────────────────────────────────────────────────────────
    {
        "Squelch",
        180.f, 1.0f, 0.88f, 0.25f, 0.70f, 0.06f, 0, 1.f, 1.f, 0.f,
        false, 0.42f, 0.50f, 0.15f,
        false, 0.375f, 0.40f, 0.30f,
        false, 0, 0.5f, 0.7f,
        130.f, 16,
        {
            {48,true, true, false}, {48,true, false,true },
            {51,true, false,false}, {48,true, true, false},
            {48,false,false,false}, {48,true, false,false},
            {51,true, false,true }, {53,true, true, false},
            {48,true, false,false}, {48,false,false,false},
            {48,true, true, false}, {55,true, false,false},
            {48,false,false,false}, {48,true, false,false},
            {51,true, false,false}, {48,true, true, false},
        }
    },

    // ── 2: Jungle Acid ───────────────────────────────────────────────────────
    {
        "Jungle Acid",
        400.f, 0.92f, 0.80f, 0.30f, 0.65f, 0.06f, 0, 1.f, 1.f, 0.f,
        false, 0.42f, 0.50f, 0.15f,
        false, 0.375f, 0.40f, 0.30f,
        false, 0, 0.5f, 0.7f,
        160.f, 16,
        {
            {45,true, true, false}, {45,true, false,true },
            {48,true, false,false}, {45,true, true, false},
            {43,true, false,false}, {45,true, false,true },
            {47,true, false,false}, {45,true, true, false},
            {45,true, false,false}, {40,true, false,true },
            {43,true, false,false}, {45,true, true, false},
            {45,true, false,false}, {43,true, false,false},
            {40,true, false,false}, {45,true, true, false},
        }
    },

    // ── 4: Square Funk ───────────────────────────────────────────────────────
    {
        "Square Funk",
        600.f, 0.70f, 0.50f, 0.40f, 0.35f, 0.07f, 1, 1.f, 1.f, 0.f,
        false, 0.42f, 0.50f, 0.15f,
        false, 0.375f, 0.40f, 0.30f,
        false, 0, 0.5f, 0.7f,
        125.f, 16,
        {
            {48,true, false,false}, {48,false,false,false},
            {48,true, false,false}, {51,true, true, false},
            {48,false,false,false}, {48,true, false,true },
            {50,true, false,false}, {48,false,false,false},
            {48,true, false,false}, {48,false,false,false},
            {55,true, true, false}, {53,true, false,false},
            {48,false,false,false}, {48,true, false,false},
            {47,true, false,false}, {48,true, false,false},
        }
    },

    // ── 6: Acid Rave ─────────────────────────────────────────────────────────
    {
        "Acid Rave",
        280.f, 0.98f, 0.95f, 0.22f, 0.80f, 0.06f, 0, 1.f, 1.f, 0.f,
        true,  0.35f, 0.45f, 0.18f,
        false, 0.375f, 0.40f, 0.30f,
        false, 0, 0.5f, 0.7f,
        138.f, 16,
        {
            {48,true, true, false}, {48,true, false,true },
            {51,true, false,false}, {51,true, true, false},
            {51,true, false,true }, {53,true, false,false},
            {55,true, true, false}, {55,true, false,false},
            {53,true, false,true }, {51,true, true, false},
            {51,true, false,false}, {48,true, false,true },
            {48,true, true, false}, {48,true, false,false},
            {46,true, false,false}, {48,true, true, false},
        }
    },

    // ── 7: Overdrive Acid ────────────────────────────────────────────────────
    {
        "Overdrive Acid",
        500.f, 0.88f, 0.70f, 0.30f, 0.55f, 0.06f, 0, 1.f, 1.f, 0.f,
        false, 0.42f, 0.50f, 0.15f,
        false, 0.375f, 0.40f, 0.30f,
        true,  2, 0.55f, 0.65f,
        133.f, 16,
        {
            {45,true, true, false}, {45,true, false,false},
            {48,true, false,true }, {50,true, true, false},
            {45,false,false,false}, {45,true, false,false},
            {45,true, true, false}, {45,true, false,true },
            {48,true, false,false}, {48,true, true, false},
            {48,false,false,false}, {45,true, false,false},
            {43,true, false,true }, {45,true, true, false},
            {45,true, false,false}, {48,true, false,false},
        }
    },

    // ── 8: Robotic Square ────────────────────────────────────────────────────
    {
        "Robotic Square",
        320.f, 0.98f, 0.40f, 0.60f, 0.45f, 0.09f, 1, 1.f, 1.f, 0.f,
        false, 0.42f, 0.50f, 0.15f,
        true,  0.25f, 0.50f, 0.25f,
        false, 0, 0.5f, 0.7f,
        128.f, 16,
        {
            {48,true, false,false}, {51,true, false,false},
            {48,true, true, false}, {48,false,false,false},
            {48,true, false,true }, {53,true, false,false},
            {51,true, true, false}, {48,true, false,false},
            {48,true, false,false}, {55,true, true, false},
            {53,true, false,true }, {51,true, false,false},
            {48,true, true, false}, {48,false,false,false},
            {46,true, false,false}, {48,true, true, false},
        }
    },

    // ── 9: Smooth Groove ─────────────────────────────────────────────────────
    {
        "Smooth Groove",
        700.f, 0.60f, 0.45f, 0.70f, 0.25f, 0.10f, 0, 1.f, 1.f, 0.f,
        true,  0.50f, 0.60f, 0.20f,
        false, 0.375f, 0.40f, 0.30f,
        false, 0, 0.5f, 0.7f,
        115.f, 16,
        {
            {45,true, false,false}, {45,false,false,false},
            {45,true, false,true }, {48,true, false,false},
            {45,false,false,false}, {43,true, false,false},
            {45,true, false,false}, {45,false,false,false},
            {45,true, false,false}, {47,true, false,true },
            {48,true, false,false}, {45,false,false,false},
            {43,true, false,false}, {45,true, false,false},
            {43,true, false,true }, {45,true, false,false},
        }
    },
};

static constexpr int NUM_FACTORY_PRESETS =
    (int)(sizeof(FACTORY_PRESETS) / sizeof(FACTORY_PRESETS[0]));
