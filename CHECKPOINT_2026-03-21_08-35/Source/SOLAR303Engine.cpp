#include "SOLAR303Engine.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SOLAR303Engine::SOLAR303Engine() = default;

void SOLAR303Engine::prepare(double sampleRate)
{
    sr = sampleRate;
    filter.prepare(sampleRate);
    reset();
}

void SOLAR303Engine::reset()
{
    phase = 0.f;
    freq = targetFreq = 440.f;
    sliding      = false;
    driftPhase   = 0.f;
    driftVal     = 0.f;
    driftTarget  = 0.f;
    envVal       = 0.f;
    vcaVal       = 0.f;
    envGate      = false;
    noteAccented = false;
    heldNotes.clear();
    filter.reset();
    // NOTE: accState is intentionally NOT reset here —
    // the 1µF capacitor retains charge across reset (analog memory)
}

// ── Helpers ───────────────────────────────────────────────────────────────────

float SOLAR303Engine::midiToHz(int note, float tuningSemitones)
{
    return 440.f * std::pow(2.f, (note - 69 + tuningSemitones) / 12.f);
}

// Standard PolyBLEP correction (band-limited step)
float SOLAR303Engine::polyBlep(float t, float dt) noexcept
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.f;
    }
    if (t > 1.f - dt)
    {
        t = (t - 1.f) / dt;
        return t * t + t + t + 1.f;
    }
    return 0.f;
}

// ── MIDI ──────────────────────────────────────────────────────────────────────

void SOLAR303Engine::noteOn(int midiNote, float velocity)
{
    const bool isLegato = !heldNotes.empty();
    heldNotes.push_back(midiNote);

    const float newFreq = midiToHz(midiNote, pTuning);

    if (isLegato)
    {
        // ── Slide: no retrigger, no phase reset, continuous state ────────
        targetFreq = newFreq;
        sliding    = true;
        // envGate stays true, envelope continues — no retrigger on legato
    }
    else
    {
        // ── New note: retrigger everything ───────────────────────────────
        freq = targetFreq = newFreq;
        sliding  = false;
        envVal   = 1.f;    // MEG fires at full
        vcaVal   = 1.f;    // VCA opens
        envGate  = true;
    }

    noteAccented = (velocity > 0.7f);
}

void SOLAR303Engine::noteOff(int midiNote)
{
    heldNotes.erase(
        std::remove(heldNotes.begin(), heldNotes.end(), midiNote),
        heldNotes.end());

    if (heldNotes.empty())
    {
        envGate = false;
    }
    else
    {
        targetFreq = midiToHz(heldNotes.back(), pTuning);
        sliding    = true;
    }
}

void SOLAR303Engine::allNotesOff()
{
    heldNotes.clear();
    envGate      = false;
    envVal       = 0.f;
    vcaVal       = 0.f;
    noteAccented = false;
    // accState NOT reset — analog memory persists
}

// ── Audio ─────────────────────────────────────────────────────────────────────

float SOLAR303Engine::processSample()
{
    const float srf = (float)sr;

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 2: SLIDE (Pitch CV)
    //  Exponential RC portamento — τ from pSlide knob (~20–300 ms)
    //  No envelope retrigger, no phase reset, continuous state
    // ══════════════════════════════════════════════════════════════════════════
    if (sliding)
    {
        const float slideTau = std::max(0.005f, pSlide);
        const float slideCoeff = std::exp(-1.f / (slideTau * srf));
        freq = targetFreq + (freq - targetFreq) * slideCoeff;

        if (std::abs(freq - targetFreq) < 0.01f)
        {
            freq = targetFreq;
            sliding = false;
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 1: VCO — TB-303 Authentic Analog Model
    //  Ramp generator (saw) + Comparator (square) — imperfect by design
    //  Based on: integrator + comparator topology with analog non-idealities
    // ══════════════════════════════════════════════════════════════════════════

    // ── 6. Drift: slow random LFO on pitch (±0.1%, ~0.3 Hz) ──────────────
    // Gives "life" and subtle movement — analog VCO is never perfectly stable
    driftPhase += 0.3f / srf;   // ~0.3 Hz LFO rate
    if (driftPhase >= 1.f)
    {
        driftPhase -= 1.f;
        driftTarget = (((float)std::rand() / (float)RAND_MAX) * 2.f - 1.f) * kDriftAmt;
    }
    driftVal += (driftTarget - driftVal) * (0.3f / srf);  // smooth interpolation
    const float driftedFreq = freq * (1.f + driftVal);

    // ── Phase accumulator ─────────────────────────────────────────────────
    const float dt = std::max(1e-6f, driftedFreq / srf);

    // ── 2. Sawtooth: imperfect ramp (curvature from analog capacitor charge)
    // y_saw = (2*phase - 1) + c * (phase - phase²)
    // c = 0.035 → subtle non-linearity, "warm" not "digital"
    float sawRaw = (2.f * phase - 1.f) + kSawCurve * (phase - phase * phase);

    // ── Anti-aliasing (PolyBLEP on saw) ───────────────────────────────────
    sawRaw -= polyBlep(phase, dt);

    // ── 3. Soft reset: transition not perfectly instantaneous ─────────────
    // When phase wraps, saw *= (1 - r) → softens the hard edge
    float osc;
    if (pSquareWave)
    {
        // ── 4. Square from Comparator: duty ≠ 50% ────────────────────────
        // d = 0.49 → slight asymmetry, not sterile
        osc = (phase > kSquareDuty) ? 1.f : -1.f;

        // PolyBLEP on square edges (at duty point and at 0/1 wrap)
        osc += polyBlep(phase, dt);
        float p2 = phase + (1.f - kSquareDuty);
        if (p2 >= 1.f) p2 -= 1.f;
        osc -= polyBlep(p2, dt);

        // ── 5. Bleed: saw leaks into square (analog coupling) ────────────
        // y_square += ε * y_saw → gives body, less sterile
        osc += kSawBleed * sawRaw;

        // ── 9. Soft clip on square output ─────────────────────────────────
        osc = std::tanh(1.2f * osc);
    }
    else
    {
        // ── SAW output with curvature (already computed above) ────────────
        osc = sawRaw;
    }

    // ── 8. Tiny noise: analog floor ───────────────────────────────────────
    // n = ±0.001 → "analogness"
    osc += (((float)std::rand() / (float)RAND_MAX) * 2.f - 1.f) * kTinyNoise;

    // ── Phase advance + soft reset ────────────────────────────────────────
    phase += dt;
    if (phase >= 1.f)
    {
        phase -= 1.f;
        // 3. Reset not ideal: slight softening at wrap point
        osc *= (1.f - kSoftReset);
    }

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 3: MAIN ENVELOPE GENERATOR (MEG)
    //  Exponential decay only — no ADSR
    //  Range: τ = 0.20s (min) to 2.5s (max), expo-mapped via APVTS
    //  On accent: forced to ~0.20s regardless of DECAY knob
    // ══════════════════════════════════════════════════════════════════════════
    if (envGate)
    {
        float decayTau;
        if (noteAccented)
            decayTau = 0.20f;   // accent forces minimum decay — constant "bite"
        else
            decayTau = pDecay;  // 0.20–2.5s from DECAY knob

        envVal *= std::exp(-1.f / (decayTau * srf));
    }
    else
    {
        // Gate off: fast release (~5 ms)
        envVal *= std::exp(-1.f / (0.005f * srf));
    }
    if (envVal < 1e-6f) envVal = 0.f;

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 9: VCA ENVELOPE
    //  Simple gate + fast release — separate from MEG
    //  VCA stays open while gate is held, then fast release
    // ══════════════════════════════════════════════════════════════════════════
    if (!envGate)
    {
        vcaVal *= std::exp(-1.f / (0.005f * srf));
    }
    if (vcaVal < 1e-6f) vcaVal = 0.f;

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 4: ACCENT SYSTEM — Three parallel paths from same MEG
    // ══════════════════════════════════════════════════════════════════════════

    // ── Path 2: Accent → VCA injection ──────────────────────────────────────
    // 47kΩ + 0.033µF RC slew:  τ = 1.55 ms
    // Gives "bite" not "click" — critical timing
    {
        const float accentTarget = (noteAccented && envGate) ? pAccent : 0.f;
        const float vcaSlewCoeff = 1.f - std::exp(-1.f / (0.00155f * srf));
        accState.vcaSlew += (accentTarget - accState.vcaSlew) * vcaSlewCoeff;
    }

    // ── Path 3: Accent Sweep → VCF (1µF capacitor) ─────────────────────────
    // Charge τ depends on resonance (Gang 2): lerp(8ms, 147ms, resonance)
    // Discharge τ ≈ 100 ms (fixed)
    // Capacitor NEVER reset — residual ~20–40% between notes = rising squeal
    {
        if (noteAccented && envGate)
        {
            // Charge: resonance-coupled timing
            const float tauCharge = 0.008f + (0.147f - 0.008f) * pResonance;
            const float chargeCoeff = 1.f - std::exp(-1.f / (tauCharge * srf));
            accState.accentCap += (1.0f - accState.accentCap) * chargeCoeff;
        }
        else
        {
            // Discharge: fixed ~100 ms
            const float dischargeCoeff = std::exp(-1.f / (0.10f * srf));
            accState.accentCap *= dischargeCoeff;
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 5: CONTROL LAW LAYER
    //  All modulation happens in log-frequency domain (anti-log converter)
    //  This is the heart of the TB-303 control system
    // ══════════════════════════════════════════════════════════════════════════

    // Convert cutoff Hz to CV (log domain: octaves above 20 Hz reference)
    float cutoffCV = std::log2(std::max(20.f, pCutoff) / 20.f);

    // ── ENV MOD: dual action (Roland's "gimmick") ───────────────────────────
    // 1. Raises envelope depth (scales MEG → VCF modulation)
    // 2. Simultaneously LOWERS cutoff base (bias shift)
    // This is why low ENV MOD = bright idle, high ENV MOD = dark idle + big sweep
    const float envModDepth = 0.3f + pEnvMod * 1.2f;   // 0.3× to 1.5× scale
    const float envModBias  = pEnvMod * 0.20f;          // 0–20% base reduction

    cutoffCV -= envModBias;                              // bias pulls base DOWN
    cutoffCV += envModDepth * envVal;                    // envelope opens filter UP

    // ── Accent Sweep → VCF (from 1µF capacitor) ────────────────────────────
    // Separate from envelope — adds on top, depth scales with pAccent
    const float accentSweepDepth = pAccent * 5.0f;      // up to 5 octaves — enough for screech
    cutoffCV += accentSweepDepth * accState.accentCap;

    // Convert back from log domain to Hz (anti-log converter output)
    float finalCutoff = 20.f * std::pow(2.f, cutoffCV);
    finalCutoff = std::max(20.f, std::min(18000.f, finalCutoff));

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 7: RESONANCE
    //  Frequency-dependent damping is handled inside DiodeLadderFilter
    //  (k_eff ≈ 0.6k at low freq, k_eff ≈ k at high freq)
    // ══════════════════════════════════════════════════════════════════════════
    const float finalRes = pResonance;

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 6: VCF CORE (Diode Ladder Filter)
    //  4 non-identical poles, tanh feedback, internal lag
    // ══════════════════════════════════════════════════════════════════════════
    const float filtered = filter.process(osc, finalCutoff, finalRes);

    // ══════════════════════════════════════════════════════════════════════════
    //  STEP 9: VCA (with accent injection)
    //  Main VCA + accent boost through RC slew
    // ══════════════════════════════════════════════════════════════════════════
    float vca = vcaVal + accState.vcaSlew;
    vca = std::min(1.5f, vca);   // allow slight boost from accent, clamp safety

    float output = filtered * vca * pVolume;

    // ══════════════════════════════════════════════════════════════════════════
    //  DISTORTION (6 types — unchanged from previous implementation)
    // ══════════════════════════════════════════════════════════════════════════
    if (pDistOn && pDistAmount > 0.001f)
    {
        const float drive = 1.0f + pDistAmount * 30.0f;
        float x = output * drive;

        switch (pDistType)
        {
            default:
            case 0: output = std::tanh(x); break;

            case 1:
                output = std::max(-1.0f, std::min(1.0f, x));
                output = output * 0.95f + std::tanh(output) * 0.05f;
                break;

            case 2:
            {
                const float px = std::max(0.f, x);
                const float nx = std::min(0.f, x);
                const float pos = 1.f - std::exp(-px * 1.8f);
                const float neg = -(1.f - std::exp(nx * 2.4f));
                output = std::max(-1.0f, std::min(1.0f, pos + neg * 0.85f));
                break;
            }

            case 3:
            {
                float t = x;
                t = std::fmod(std::abs(t + 1.f), 4.f);
                if (t > 2.f) t = 4.f - t;
                output = t - 1.f;
                output = std::tanh(output * 1.4f);
                break;
            }

            case 4:
                if (x >= 0.f)
                    output = 1.f - 1.f / (1.f + x * 2.5f);
                else
                    output = -(1.f - 1.f / (1.f - x * 1.4f));
                output *= 1.6f;
                output = std::max(-1.5f, std::min(1.5f, output));
                break;

            case 5:
            {
                const int bits = std::max(2, (int)(8.f - pDistAmount * 6.f));
                const float step = 2.f / std::pow(2.f, (float)bits);
                output = std::round(x / step) * step;
                output = std::max(-1.5f, std::min(1.5f, output));
                break;
            }
        }

        output *= pDistVolume;
    }

    return output;
}
