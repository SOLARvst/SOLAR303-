#pragma once
#include <JuceHeader.h>

// ── SOLAR303 Skin Definition ──────────────────────────────────────────────────
struct SOLAR303Skin
{
    juce::String name;
    juce::Colour gradientTop;     // chassis gradient — top colour
    juce::Colour gradientBottom;  // chassis gradient — bottom colour
    juce::Colour chassisText;     // row labels (NOTE / ACC / SLD / STEP / ML)
};

// ── Built-in Skins ────────────────────────────────────────────────────────────
namespace SOLAR303Skins
{
    // 1. Red (default) — Roland red → black
    static const SOLAR303Skin Red {
        "Red",
        juce::Colour(0xffc42c0c),   // RolandRed
        juce::Colour(0xff000000),   // black
        juce::Colour(0xffe0ddd8),   // light text
    };

    // 2. Dark Gray — matches original pre-red chassis
    static const SOLAR303Skin DarkGray {
        "Dark Gray",
        juce::Colour(0xff525050),   // dark gray top
        juce::Colour(0xff1a1818),   // near-black bottom
        juce::Colour(0xffe0ddd8),   // light text
    };

    // 3. Light Gray — classic TB-303 silver-ish
    static const SOLAR303Skin LightGray {
        "Light Gray",
        juce::Colour(0xffc8c5c0),   // silver-gray top
        juce::Colour(0xff525050),   // dark gray bottom
        juce::Colour(0xff101010),   // dark text (readable on light bg)
    };

    // 4. Yellow — acid yellow → black
    static const SOLAR303Skin Yellow {
        "Yellow",
        juce::Colour(0xffddaa00),   // amber-yellow top
        juce::Colour(0xff000000),   // black bottom
        juce::Colour(0xffe0ddd8),   // light text
    };

    // 5. Black — all-black stealth
    static const SOLAR303Skin Black {
        "Black",
        juce::Colour(0xff252525),   // very dark gray top
        juce::Colour(0xff000000),   // black bottom
        juce::Colour(0xffe0ddd8),   // light text
    };

    // 6. Blue Purple — blue-tinted purple → near-black
    static const SOLAR303Skin BluePurple {
        "BLUERPEL",
        juce::Colour(0xff3a1a7a),   // deep blue-purple top
        juce::Colour(0xff0a0418),   // near-black bottom
        juce::Colour(0xffe0ddd8),   // light text
    };

    // ── Ordered list (matches popup menu order) ───────────────────────────────
    static inline std::vector<SOLAR303Skin> All()
    {
        return { Red, DarkGray, LightGray, Yellow, Black, BluePurple };
    }
}
