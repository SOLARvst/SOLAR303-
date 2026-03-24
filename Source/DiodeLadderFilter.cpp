#include "DiodeLadderFilter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DiodeLadderFilter::DiodeLadderFilter() { reset(); }

void DiodeLadderFilter::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
}

void DiodeLadderFilter::reset()
{
    s[0] = s[1] = s[2] = s[3] = 0.f;
    cutoffSmooth = 800.f;
}

float DiodeLadderFilter::process(float input, float cutoffHz, float resonance)
{
    if (cutoffHz < 20.f)    cutoffHz = 20.f;
    if (cutoffHz > 18000.f) cutoffHz = 18000.f;
    if (resonance < 0.f)   resonance = 0.f;
    if (resonance > 1.f)   resonance = 1.f;

    // ── Internal cutoff lag (~0.5 ms) — reduces digital stepping artifacts ──
    const float lagCoeff = 1.f - std::exp(-1.f / (0.0005f * (float)sr));
    cutoffSmooth += (cutoffHz - cutoffSmooth) * lagCoeff;

    // 4× internal oversampling for numerical stability
    constexpr int OS = 4;
    const float f = (float)(2.0 * M_PI * (double)cutoffSmooth / (sr * OS));

    // ── Non-identical pole coefficients (TB-303 component tolerances) ────
    const float g0 = f;           // pole 1: nominal
    const float g1 = f * 0.90f;   // pole 2: slightly slower
    const float g2 = f * 1.10f;   // pole 3: slightly faster
    const float g3 = f * 0.95f;   // pole 4: slightly slower

    // ── Frequency-dependent resonance ───────────────────────────────────
    // TB-303: resonance is weaker in bass, stronger in treble
    // At low cutoff: k_eff ≈ 0.6k,  at high cutoff: k_eff ≈ k
    const float gNorm = f / (f + 0.02f);   // 0→low freq, 1→high freq
    const float kScale = 0.6f + 0.4f * gNorm;
    const float k = resonance * 3.9f * kScale;

    float out = 0.f;
    for (int i = 0; i < OS; ++i)
    {
        // Feedback with diode-style saturation at the input
        const float x = tanhApprox(input * 1.2f - k * tanhApprox(s[3]));

        // Four cascaded integrator stages — non-identical (Euler, per-stage saturation)
        s[0] += g0 * (x              - tanhApprox(s[0]));
        s[1] += g1 * (tanhApprox(s[0]) - tanhApprox(s[1]));
        s[2] += g2 * (tanhApprox(s[1]) - tanhApprox(s[2]));
        s[3] += g3 * (tanhApprox(s[2]) - tanhApprox(s[3]));

        out = s[3];
    }
    return out;
}
