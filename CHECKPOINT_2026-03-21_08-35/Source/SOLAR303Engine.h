#pragma once
#include "DiodeLadderFilter.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>

/**
 * TB-303 Bass Synthesizer Engine — Authentic Analog Model
 *
 * Architecture (Roland Service Notes + Robin Whittle analysis):
 *   MIDI → VCO (Saw / Square + PolyBLEP)
 *        → Anti-Log Control Law (log-domain CV modulation)
 *        → Diode Ladder Filter (4 non-identical poles)
 *        → VCA (with accent injection)
 *        → Output
 *
 * Authentic features modelled:
 *   - PolyBLEP anti-aliased sawtooth and square oscillators
 *   - Anti-log converter: all modulation in log-frequency domain
 *   - Exponential AD envelope (no ADSR) with accent-forced minimum decay
 *   - 3-path accent system: MEG override + VCA slew + VCF sweep accumulation
 *   - 1uF accent sweep capacitor: NEVER reset between notes (analog memory)
 *   - Dual-gang resonance: Gang 1 = filter Q, Gang 2 = accent sweep timing
 *   - Env Mod: raises depth AND lowers cutoff base (Roland bias shift gimmick)
 *   - Slide: 60ms exponential RC portamento, no retrigger
 *   - Frequency-dependent resonance (weaker in bass)
 *   - Non-identical filter poles (component tolerance)
 */
class SOLAR303Engine
{
public:
    SOLAR303Engine();

    void prepare(double sampleRate);
    void reset();

    // Called from audio thread — returns one output sample
    float processSample();

    // MIDI (called from audio thread inside processBlock)
    void noteOn (int midiNote, float velocity);   // velocity 0.0–1.0
    void noteOff(int midiNote);
    void allNotesOff();

    // ── Parameters (written from processBlock before rendering) ──────────────
    float pCutoff     = 800.f;   // Hz  (60–5000) — via anti-log converter
    float pResonance  = 0.5f;    // 0–1  (dual-gang: Q + accent sweep timing)
    float pEnvMod     = 0.5f;    // 0–1  (envelope depth + cutoff bias shift)
    float pDecay      = 0.5f;    // seconds (0.20–2.5) — expo envelope tau, skew 0.35
    float pAccent     = 0.5f;    // 0–1  (accent intensity: VCA + VCF sweep)
    float pSlide      = 0.06f;   // seconds (0.02–0.3) — RC portamento tau, default 60ms
    float pVolume     = 0.8f;    // 0–1
    bool  pSquareWave = false;   // false = saw,  true = square
    float pTuning     = 0.f;     // semitones (-24 to +24)

    // ── FX: Reverb ────────────────────────────────────────────────────────────
    bool  pRevOn   = false;
    float pRevSize = 0.5f;    // 0–1  room size
    float pRevDamp = 0.5f;    // 0–1  high-freq damping
    float pRevMix  = 0.25f;   // 0–1  wet/dry

    // ── FX: Delay ─────────────────────────────────────────────────────────────
    bool  pDelayOn   = false;
    float pDelayTime = 0.375f;  // seconds (0.02–2.0)
    float pDelayFeed = 0.4f;    // 0–0.9 feedback
    float pDelayMix  = 0.3f;    // 0–1  wet/dry

    // ── Distortion ────────────────────────────────────────────────────────────
    bool  pDistOn     = false;
    int   pDistType   = 0;       // 0=Tanh  1=Hard  2=Fuzz  3=Fold  4=Tube  5=Crush
    float pDistAmount = 0.5f;    // 0–1
    float pDistVolume = 0.7f;    // 0–1

private:
    double sr = 44100.0;

    // ── VCO (TB-303 authentic: imperfect ramp + dirty comparator) ─────────
    float phase       = 0.f;
    float freq        = 440.f;
    float targetFreq  = 440.f;
    bool  sliding     = false;

    // Drift: slow random LFO on pitch (±0.05–0.2%, ~0.1–1 Hz)
    float driftPhase  = 0.f;     // phase of drift LFO
    float driftVal    = 0.f;     // current drift offset (smoothed)
    float driftTarget = 0.f;     // next random target for drift

    // VCO imperfection constants (TB-303 spec values)
    static constexpr float kSawCurve   = 0.035f;   // saw curvature: 2–5% → 3.5%
    static constexpr float kSoftReset  = 0.03f;    // reset softness: 2–5% → 3%
    static constexpr float kSquareDuty = 0.49f;    // duty cycle: 48–52% → 49%
    static constexpr float kSawBleed   = 0.02f;    // saw→square bleed: 1–3% → 2%
    static constexpr float kDriftAmt   = 0.001f;   // drift: ±0.1%
    static constexpr float kTinyNoise  = 0.001f;   // output noise floor

    // ── Main Envelope Generator (MEG) ───────────────────────────────────────
    // Exponential decay only (no ADSR) — drives filter cutoff modulation
    float envVal      = 0.f;     // 0–1,  drives VCF via ENV MOD
    bool  envGate     = false;   // true while any note is held

    // ── VCA Envelope ────────────────────────────────────────────────────────
    // Simple gate + fast release — separate from MEG
    float vcaVal      = 0.f;     // 0–1,  main VCA level

    // ── Accent subsystem (authentic TB-303 topology) ────────────────────────
    // Three parallel paths driven by the SAME main envelope (MEG):
    //   Path 1: MEG decay forced to minimum (~0.20s) on accent
    //   Path 2: Accent → VCA injection through 47k + 0.033µF RC slew (τ=1.55ms)
    //   Path 3: Accent Sweep → VCF through 1µF cap with resonance-coupled timing
    // The 1µF cap state is NEVER reset between notes — analog memory creates
    // rising squeal on consecutive accents.
    struct AccentState {
        float accentCap = 0.f;   // 1µF capacitor — accent sweep analog memory
        float vcaSlew   = 0.f;   // 47k + 0.033µF RC smoothing for VCA injection
    };
    AccentState accState;
    bool  noteAccented = false;  // true while current note has accent flag

    // ── Note tracking (for legato / slide detection) ──────────────────────
    std::vector<int> heldNotes;

    // ── Filter ───────────────────────────────────────────────────────────────
    DiodeLadderFilter filter;

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float midiToHz(int note, float tuningSemitones);
    static float polyBlep(float phase, float dt) noexcept;
};
