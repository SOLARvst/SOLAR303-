#pragma once
#include <cmath>
#include <algorithm>

/**
 * Nonlinear Diode Ladder Filter — Roland TB-303 authentic model
 *
 * Features:
 *   - 4 non-identical pole stages (g, 0.9g, 1.1g, 0.95g)
 *   - Frequency-dependent resonance (weaker in bass, no true self-oscillation)
 *   - Internal cutoff smoothing (0.5 ms lag — reduces digital artifacts)
 *   - tanh saturation at each stage (diode non-linearity)
 *   - 4× oversampled for numerical stability
 */
class DiodeLadderFilter
{
public:
    DiodeLadderFilter();
    void prepare(double sampleRate);
    void reset();

    // cutoffHz: 20–18000 Hz  |  resonance: 0.0–1.0
    float process(float input, float cutoffHz, float resonance);

private:
    double sr = 44100.0;
    float  s[4] = { 0.f, 0.f, 0.f, 0.f };
    float  cutoffSmooth = 800.f;   // internal lag state

    // Padé approximation of tanh — fast and accurate to ±0.5 % for |x| < 4
    static inline float tanhApprox(float x) noexcept
    {
        if (x >  4.f) return  1.f;
        if (x < -4.f) return -1.f;
        const float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }
};
