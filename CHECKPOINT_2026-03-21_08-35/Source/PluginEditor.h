#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SOLAR303Sequencer.h"
#include "Skins/SkinManager.h"

// ── TB-303 Original Colour Palette (Roland Bass Line, 1981) ──────────────────
namespace AcidColors
{
    // ── Chassis ──────────────────────────────────────────────────────────────
    const juce::Colour Background  { 0xff8b1a0a };  // dark red chassis
    const juce::Colour Panel       { 0xff8b1a0a };  // same — unified
    const juce::Colour PanelDark   { 0xff5a0f06 };  // darker red

    // ── Roland brand ─────────────────────────────────────────────────────────
    const juce::Colour RolandRed   { 0xffc42c0c };  // Roland red-orange
    const juce::Colour RolandRedDim{ 0xff6a1808 };

    // ── Button caps (dark charcoal) ───────────────────────────────────────────
    const juce::Colour BtnOff      { 0xff1a1816 };  // button cap, inactive
    const juce::Colour BtnHi       { 0xff2e2c28 };  // button cap, hover
    const juce::Colour BtnRim      { 0xff3a3834 };  // button border

    // ── LED colours ──────────────────────────────────────────────────────────
    const juce::Colour LedOff      { 0xff0c0b0a };  // LED unlit (almost black)
    const juce::Colour LedAmber    { 0xffff9900 };  // step LED on  (original amber)
    const juce::Colour LedAmberDim { 0xff4a2c00 };  // step LED off (dark amber)
    const juce::Colour LedGreen    { 0xff00cc20 };  // play LED on
    const juce::Colour LedRed      { 0xffdd1010 };  // accent LED on
    const juce::Colour LedBlue     { 0xff00aadd };  // slide LED on
    const juce::Colour LedWhite    { 0xffffffff };  // current-step strobe

    // ── Text ─────────────────────────────────────────────────────────────────
    const juce::Colour TextDark    { 0xff101010 };  // labels on gray chassis
    const juce::Colour TextDim     { 0xff606058 };  // secondary/muted labels
    const juce::Colour TextOnBtn   { 0xffe0ddd8 };  // text on dark buttons

    // ── Knobs ────────────────────────────────────────────────────────────────
    const juce::Colour KnobBody    { 0xff0e0c0a };  // knob cap
    const juce::Colour KnobRim     { 0xff484440 };  // knob track ring
    const juce::Colour KnobArc     { 0xffff3300 };  // value arc (bright red-orange)

    // ── Legacy aliases (keep existing code compiling) ─────────────────────────
    const juce::Colour Yellow      = LedAmber;
    const juce::Colour YellowBright= LedWhite;
    const juce::Colour YellowDim   = LedAmberDim;
    const juce::Colour Orange      = RolandRed;
    const juce::Colour OrangeDim   = RolandRedDim;
    const juce::Colour StepOn      = LedAmber;
    const juce::Colour StepOff     = LedOff;
    const juce::Colour StepActive  = LedWhite;
    const juce::Colour AccentOn    = LedRed;
    const juce::Colour SlideOn     = LedBlue;
    const juce::Colour TextBright  = TextOnBtn;
}

// ── Custom LookAndFeel ────────────────────────────────────────────────────────
class AcidLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AcidLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour&, bool highlighted, bool down) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool highlighted, bool down) override;

    juce::Font getLabelFont(juce::Label&) override;
};

// ── Note button — hold = organ sustain, arrow click = note-picker popup ───────
class NoteButton : public juce::TextButton
{
public:
    int stepIndex = 0;
    std::function<void(int step, int delta)> onNoteChange;   // scroll wheel / right-click
    std::function<void(int step, int note)>  onNotePicked;   // popup selection
    std::function<void(int step)>            onPreviewStart; // press  → noteOn
    std::function<void(int step)>            onPreviewStop;  // release → noteOff

    void mouseWheelMove(const juce::MouseEvent&,
                        const juce::MouseWheelDetails& w) override
    {
        if (onNoteChange) onNoteChange(stepIndex, w.deltaY > 0 ? 1 : -1);
        if (onPreviewStart) onPreviewStart(stepIndex);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // ── noteOn immediately — note sustains until mouseUp (organ) ─────────
        if (onPreviewStart) onPreviewStart(stepIndex);

        if (e.mods.isRightButtonDown())
        {
            arrowClicked = false;
            // Right-click: semitone down (auditioned above)
            if (onNoteChange) onNoteChange(stepIndex, -1);
            return;
        }

        // Popup only if click landed on the small arrow (bottom-right 14×14 px)
        const auto b = getLocalBounds();
        arrowClicked = (e.x >= b.getWidth()  - 14 &&
                        e.y >= b.getHeight() - 14);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        juce::TextButton::mouseUp(e);

        // ── noteOff on release (organ behaviour) ─────────────────────────────
        if (onPreviewStop) onPreviewStop(stepIndex);

        if (!arrowClicked) return;   // body press = pure note preview, no popup
        arrowClicked = false;

        // Arrow was clicked → open note-picker popup
        static const char* names[] =
            {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

        juce::PopupMenu menu;
        for (int oct = 1; oct <= 6; ++oct)
        {
            juce::PopupMenu sub;
            for (int n = 0; n < 12; ++n)
            {
                const int midi = (oct + 1) * 12 + n;   // C1=24 … C6=84
                if (midi > 84) break;
                sub.addItem(midi + 1,
                    juce::String(names[n]) + juce::String(oct));
            }
            menu.addSubMenu(juce::String("Oct ") + juce::String(oct), sub);
        }

        menu.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(this),
            [this](int result)
            {
                if (result > 0)
                {
                    if (onNotePicked) onNotePicked(stepIndex, result - 1);
                    // Audition the newly picked note briefly
                    if (onPreviewStart) onPreviewStart(stepIndex);
                    if (onPreviewStop)  onPreviewStop (stepIndex);
                }
            });
    }

private:
    bool arrowClicked = false;
};

// ── MIDI-learnable slider ─────────────────────────────────────────────────────
class MidiSlider : public juce::Slider
{
public:
    MidiSlider()
    {
        setWantsKeyboardFocus(false);
        setMouseClickGrabsKeyboardFocus(false);
    }

    std::function<void(juce::Component*)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            if (onRightClick) onRightClick(this);
            return;   // suppress Slider's own right-click popup
        }
        juce::Slider::mouseDown(e);
    }

    // Block all keyboard input — arrow keys must NOT move knobs
    bool keyPressed(const juce::KeyPress&) override { return false; }
};

// ── MIDI drag button — drag onto a DAW track to export the pattern as MIDI ───
class MidiDragButton : public juce::TextButton
{
public:
    std::function<void()> onDragStarted;
    std::function<void()> onRightClick;   // right-click → popup menu (Export / Import)

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            if (onRightClick) onRightClick();
            return;   // suppress default right-click behaviour
        }
        juce::TextButton::mouseDown(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (!dragging && onDragStarted && e.getDistanceFromDragStart() > 5)
        {
            dragging = true;
            onDragStarted();   // blocks until OS drag completes
            dragging = false;
        }
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        juce::TextButton::mouseUp(e);
        dragging = false;
    }

private:
    bool dragging = false;
};

// ── Main Editor ───────────────────────────────────────────────────────────────
class SOLAR303Editor : public juce::AudioProcessorEditor,
                    public juce::Timer,
                    public juce::KeyListener
{
public:
    explicit SOLAR303Editor(SOLAR303Processor&);
    ~SOLAR303Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void setScaleFactor(float newScale) override;
    bool keyPressed(const juce::KeyPress& key) override;

    // KeyListener interface — catches keys even without focus
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override
    {
        return keyPressed(key);
    }
    bool keyStateChanged(bool, juce::Component*) override { return false; }

private:
    SOLAR303Processor& processor;
    AcidLookAndFeel laf;

    // ── Sequencer controls ────────────────────────────────────────────────────
    juce::TextButton playButton        { "PLAY" };
    juce::TextButton skinButton        { "SKIN" };   // skin selector
    juce::TextButton randomButton      { "REND A" };  // randomises steps + knobs
    juce::TextButton randomKnobButton  { "REND K" };  // randomises knobs only
    juce::TextButton randomStepsButton { "REND S" };  // randomises steps only, knobs untouched
    juce::TextButton rndBackButton     { "<" };      // navigate to previous random state
    juce::TextButton rndFwdButton      { ">" };      // navigate to next random state
    juce::TextButton resetKnobsButton;
    bool             resetKnobsTo50 { false };
    juce::Slider     bpmSlider;
    juce::Label      bpmLabel;
    juce::Slider     lengthSlider;
    juce::Label      lengthLabel;
    MidiDragButton   dragMidiButton;                   // VST3 only
    juce::TextButton importPresetButton { "IMPORT" }; // VST3 only
    juce::TextButton exportPresetButton { "EXPORT" }; // VST3 only
    // Standalone preset buttons — right-click → "Show in File Explorer"
    struct PresetButton : public juce::TextButton
    {
        using juce::TextButton::TextButton;
        std::function<void()> onRightClick;
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (e.mods.isRightButtonDown()) { if (onRightClick) onRightClick(); return; }
            juce::TextButton::mouseDown(e);
        }
    };
    PresetButton savePresetButton    { "SAVE" };          // Standalone only
    PresetButton loadPresetButton    { "LOAD" };          // Standalone only
    juce::TextButton showExplorerButton { "EXPLORER" };   // Standalone only
    std::unique_ptr<juce::FileChooser> fileChooser;   // kept alive during async dialog

    // ── HI-PASS FX section — both VST3 and Standalone ────────────────────────
    juce::TextButton hpfOnButton      { "HPF OFF" }; // ON/OFF toggle
    juce::TextButton hpfFreqButtons[5];              // radio: 30/50/100/125/150 Hz
    juce::TextButton hpf24Button      { "24 dB" };   // 24 dB/oct slope
    juce::TextButton hpf48Button      { "48 dB" };   // 48 dB/oct slope
    int  hpfFreqState = 0;   // 0=30Hz, 1=50Hz, 2=100Hz, 3=125Hz, 4=150Hz
    int  hpfSlopeIdx  = 0;   // 0=24dB/oct, 1=48dB/oct
    bool hpfActive    = false;

    // Blue-border + blue-text when active, dim otherwise
    void setHpfBtnActive(juce::TextButton& btn, bool active)
    {
        btn.getProperties().set("activeBorder", active);
        btn.setColour(juce::TextButton::textColourOffId,
                      active ? AcidColors::LedGreen
                             : AcidColors::TextOnBtn.withAlpha(0.55f));
        btn.repaint();
    }

    // ── Step grid (16 steps each) ─────────────────────────────────────────────
    NoteButton       noteButtons  [16];
    juce::TextButton accentButtons[16];
    juce::TextButton slideButtons [16];
    juce::TextButton activeButtons[16];

    int displayedStep = -1;

    // ── Synth knobs ───────────────────────────────────────────────────────────
    MidiSlider sCutoff, sResonance, sEnvMod, sDecay, sAccent;
    MidiSlider sSlide,  sTuning;
    juce::Label  lCutoff, lResonance, lEnvMod, lDecay, lAccent;
    juce::Label  lSlide,  lTuning;

    // ── ML buttons — UP + DOWN per knob ─────────────────────────────────────
    juce::TextButton mlUpButtons  [6];   // ↑ button per knob
    juce::TextButton mlDownButtons[6];   // ↓ button per knob
    int  mlLearningKnob  { -1 };   // which knob is learning (-1 = none)
    bool mlLearningIsUp  { true }; // true = learning UP, false = learning DOWN
    int  mlKeyUp  [6]    { 0,0,0,0,0,0 };
    int  mlKeyDown[6]    { 0,0,0,0,0,0 };
    void setupMidiLearnButtons();
    juce::Slider*  getSliderForMl(int index);
    juce::String   keyCodeName(int keyCode);
    juce::ComboBox waveCombo;
    juce::Label    lWave;

    // ── Master Volume ─────────────────────────────────────────────────────────
    MidiSlider sMasterVol;
    juce::Label  lMasterVol;

    // ── FX: Reverb ────────────────────────────────────────────────────────────
    juce::TextButton revOnButton   { "REV OFF" };
    juce::Label      lRevOn;
    MidiSlider       sRevSize, sRevDamp, sRevMix;
    juce::Label      lRevSize, lRevDamp, lRevMix;

    // ── FX: Delay ─────────────────────────────────────────────────────────────
    juce::TextButton delayOnButton { "DLY OFF" };
    juce::Label      lDelayOn;
    MidiSlider       sDelayTime, sDelayFeed, sDelayMix;
    juce::Label      lDelayTime, lDelayFeed, lDelayMix;
    // Note-division presets: [0..3]=Straight, [4..7]=Triplet, [8..11]=Dotted
    juce::TextButton delayNoteButtons[12];

    // ── Distortion ────────────────────────────────────────────────────────────
    juce::TextButton distOnButton  { "DIST OFF" };
    juce::ComboBox   distTypeCombo;
    juce::Label      lDistType;
    MidiSlider       sDistAmount, sDistVol;
    juce::Label      lDistAmount,  lDistVol;

    // ── APVTS attachments ──────────────────────────────────────────────────────
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BoolAtt   = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAtt> aMasterVol;
    std::unique_ptr<SliderAtt> aCutoff, aResonance, aEnvMod, aDecay, aAccent;
    std::unique_ptr<SliderAtt> aSlide, aTuning;
    std::unique_ptr<SliderAtt> aRevSize, aRevDamp, aRevMix;
    std::unique_ptr<BoolAtt>   aRevOn;
    std::unique_ptr<SliderAtt> aDelayTime, aDelayFeed, aDelayMix;
    std::unique_ptr<BoolAtt>   aDelayOn;
    std::unique_ptr<SliderAtt> aDistAmount, aDistVol;
    std::unique_ptr<ComboAtt>  aWave, aDistType;
    std::unique_ptr<BoolAtt>   aDistOn;

    // ── Skin system ────────────────────────────────────────────────────────────
    SOLAR303Skin currentSkin { SOLAR303Skins::Red };
    void applySkin(const SOLAR303Skin& skin);
    float uiScale { 1.25f };
    float hostScale { 1.0f };

    // ── Helpers ────────────────────────────────────────────────────────────────
    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& name);
    void updateNoteButton(int step);

public:
    void refreshStepGrid();   // called from setStateInformation callAsync

private:
    void          randomiseAll();    // RND    — steps + knobs
    void          randomiseKnobs();  // REND 2 — knobs only, steps untouched
    void          randomiseSteps();  // REND 3 — steps only, knobs untouched

    // ── RND history (undo/redo for randomise operations) ──────────────────────
    struct PatternSnapshot
    {
        struct StepData { int note; bool accent, slide, active; };
        StepData steps[16];
        float cutoff, resonance, envmod, decay, accentVal, slideVal;
    };
    std::vector<PatternSnapshot> rndHistory;
    int                          rndHistIdx { -1 };

    PatternSnapshot captureSnapshot();
    void            applySnapshot   (const PatternSnapshot&);
    void            pushRndSnapshot ();           // call before randomize
    void            finaliseRndSnapshot();        // call after  randomize
    void            updateRndNavButtons();
    juce::Label*  getLabelForParam (const juce::String& paramId);
    juce::Slider* getSliderForParam(const juce::String& paramId);
    void          exportAndDragMidi();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SOLAR303Editor)
};
