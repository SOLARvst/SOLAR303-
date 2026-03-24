#include "PluginEditor.h"
#include <BinaryData.h>
#if JUCE_WINDOWS
 #define NOMINMAX       // prevent windows.h from defining min/max macros
 #include <shlobj.h>   // SHCreateStdEnumFmtEtc, DROPFILES, CF_HDROP
 #include <ole2.h>     // DoDragDrop, OleDuplicateData, IDataObject, IDropSource
#endif

using namespace AcidColors;

// ── Embedded fonts (loaded once, used everywhere) ─────────────────────────────
static juce::Typeface::Ptr getEasyRideFont()
{
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor(
        BinaryData::EasyRideExpand_ttf, BinaryData::EasyRideExpand_ttfSize);
    return tf;
}

static juce::Font easyRide(float size)
{
    return juce::Font(getEasyRideFont()).withHeight(size);
}

static juce::Typeface::Ptr getDSEG7Font()
{
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor(
        BinaryData::DSEG7ClassicBold_ttf, BinaryData::DSEG7ClassicBold_ttfSize);
    return tf;
}

static juce::Font dseg7(float size)
{
    return juce::Font(getDSEG7Font()).withHeight(size);
}

static juce::Typeface::Ptr getAcakadutFont()
{
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor(
        BinaryData::aAcakadut_ttf, BinaryData::aAcakadut_ttfSize);
    return tf;
}

static juce::Font acakadut(float size)
{
    return juce::Font(getAcakadutFont()).withHeight(size);
}

// ── AcidLookAndFeel ───────────────────────────────────────────────────────────

AcidLookAndFeel::AcidLookAndFeel()
{
    // Sliders — dark text box on gray chassis
    setColour(juce::Slider::textBoxTextColourId,       TextOnBtn);
    setColour(juce::Slider::textBoxBackgroundColourId, BtnOff);
    setColour(juce::Slider::textBoxOutlineColourId,    KnobRim);
    // Labels
    setColour(juce::Label::textColourId,               TextOnBtn);
    // ComboBox — dark cap style
    setColour(juce::ComboBox::backgroundColourId,      BtnOff);
    setColour(juce::ComboBox::textColourId,            TextOnBtn);
    setColour(juce::ComboBox::outlineColourId,         BtnRim);
    setColour(juce::ComboBox::arrowColourId,           LedGreen);
    // PopupMenu
    setColour(juce::PopupMenu::backgroundColourId,     BtnOff);
    setColour(juce::PopupMenu::textColourId,           TextOnBtn);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, BtnHi);
    setColour(juce::PopupMenu::highlightedTextColourId, LedGreen);
}

void AcidLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    const float r  = (float)juce::jmin(w, h) * 0.5f - 4.f;
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;
    const float a  = startAngle + sliderPos * (endAngle - startAngle);

    // Outer shadow ring
    g.setColour(juce::Colour(0x40000000));
    g.fillEllipse(cx - r - 1.f, cy - r - 1.f, (r + 1.f) * 2.f, (r + 1.f) * 2.f);

    // Track arc (dimmed amber)
    juce::Path track;
    track.addArc(cx - r, cy - r, r * 2.f, r * 2.f, startAngle, endAngle, true);
    g.setColour(KnobRim);
    g.strokePath(track, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Value arc — gradient: dark red at start (bottom-left) → bright red at tip (top-right)
    juce::Path val;
    val.addArc(cx - r, cy - r, r * 2.f, r * 2.f, startAngle, a, true);
    g.setGradientFill(juce::ColourGradient(
        KnobArc.darker (0.45f), cx - r, cy + r,   // start: darker red (bottom-left)
        KnobArc.brighter(0.4f), cx + r, cy - r,   // tip:   bright orange-red (top-right)
        false));
    g.strokePath(val, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));

    // Knob body — gradient: black (top) → KnobArc red (bottom)
    const float br = r - 5.f;
    g.setGradientFill(juce::ColourGradient(
        KnobBody,  cx, cy - br,        // top: near black
        KnobArc,   cx, cy + br, false));// bottom: orange-red (#ff3300)
    g.fillEllipse(cx - br, cy - br, br * 2.f, br * 2.f);
    // outer rim highlight
    g.setColour(juce::Colour(0x50ffffff));
    g.drawEllipse(cx - br, cy - br, br * 2.f, br * 2.f, 1.5f);
    // inner bottom shadow
    g.setColour(juce::Colour(0x40000000));
    g.drawEllipse(cx - br + 1.f, cy - br + 1.f, br * 2.f - 2.f, br * 2.f - 2.f, 0.8f);

    // Indicator dot (amber LED)
    const float lx = cx + (br - 5.f) * std::sin(a);
    const float ly = cy - (br - 5.f) * std::cos(a);
    g.setColour(KnobArc);
    g.drawLine(cx, cy, lx, ly, 2.f);
    g.fillEllipse(lx - 2.5f, ly - 2.5f, 5.f, 5.f);
}

void AcidLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
    const juce::Colour&, bool highlighted, bool down)
{
    auto bounds = b.getLocalBounds().toFloat().reduced(1.f);
    const bool on          = b.getToggleState() || down;
    const bool isDelayNote = b.getProperties().contains("delaynote");
    const bool lit         = highlighted || (isDelayNote && on);

    // ── Button body (strong 3D gradient) ─────────────────────────────────────
    const juce::Colour baseCol  = highlighted ? BtnHi : BtnOff;
    const juce::Colour topCol   = down ? baseCol.darker(0.7f)   : baseCol.brighter(0.85f);
    const juce::Colour botCol   = down ? baseCol.brighter(0.3f) : baseCol.darker(0.65f);
    g.setGradientFill(juce::ColourGradient(topCol, bounds.getX(), bounds.getY(),
                                           botCol, bounds.getX(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 2.5f);
    // Top bevel highlight (bright edge)
    g.setColour(juce::Colour(down ? 0x10ffffff : 0x80ffffff));
    g.fillRoundedRectangle(bounds.withHeight(3.5f), 2.5f);
    // Bottom bevel shadow (dark edge)
    g.setColour(juce::Colour(down ? 0x80000000 : 0x60000000));
    g.fillRoundedRectangle(bounds.withTop(bounds.getBottom() - 3.5f), 2.5f);

    // ── Rim ───────────────────────────────────────────────────────────────────
    const bool activeBorder  = (bool)b.getProperties().getWithDefault("activeBorder", false);
    if      (activeBorder)  g.setColour(LedGreen);
    else if (lit)           g.setColour(LedGreen);
    else                    g.setColour(BtnRim);
    g.drawRoundedRectangle(bounds, 2.5f, 1.5f);

    // ── LED indicator (small dot / filled area inside button) ────────────────
    if (b.getProperties().contains("note"))
    {
        // Note row: white strobe on current step
        if (on)
        {
            auto led = bounds.reduced(4.f, 5.f);
            g.setColour(LedWhite.withAlpha(0.85f));
            g.fillRoundedRectangle(led, 2.f);
        }

        // Small downward arrow in the bottom-right corner (dropdown hint)
        {
            const float ax = bounds.getRight() - 7.f;
            const float ay = bounds.getBottom() - 6.f;
            juce::Path arrow;
            arrow.addTriangle(ax - 4.f, ay - 2.f,
                              ax + 4.f, ay - 2.f,
                              ax,       ay + 3.f);
            g.setColour(LedGreen.withAlpha(on ? 0.5f : 0.7f));
            g.fillPath(arrow);
        }
        return;
    }

    // For all other buttons: fill interior with LED color
    auto interior = bounds.reduced(2.f);

    if (b.getProperties().contains("mlState"))
    {
        const int mlState = (int)b.getProperties()["mlState"];
        switch (mlState)
        {
            case 4:  g.setColour(LedGreen);                      break;  // learning — bright green (blink on)
            case 2:  g.setColour(LedAmber);                      break;  // learning — bright amber
            case 1:  g.setColour(LedAmberDim);                   break;  // learning — dim (blink off)
            case 3:  g.setColour(juce::Colour(0xff003300));       break;  // mapped — dark green bg
            default: g.setColour(LedOff);                        break;  // idle
        }
    }
    else if (b.getProperties().contains("accent"))
        g.setColour(on ? LedGreen : LedOff);
    else if (b.getProperties().contains("slide"))
        g.setColour(on ? LedBlue  : LedOff);
    else if (b.getProperties().contains("active"))
        g.setColour(on ? LedRed   : LedOff);
    else if (b.getProperties().contains("play"))
        g.setColour(LedOff);  // black background always
    else if (b.getProperties().contains("lockbtn"))
        g.setColour(on ? RolandRed.brighter(0.25f) : LedOff);
    else if (b.getProperties().contains("distbtn"))
        g.setColour(on ? RolandRed : LedOff);
    else if (b.getProperties().contains("dist"))
        g.setColour(on ? RolandRed : LedOff);
    else if (b.getProperties().contains("delaynote"))
        g.setColour(LedOff);
    else
        g.setColour(on ? LedGreen.withAlpha(0.35f) : LedOff);

    g.fillRoundedRectangle(interior, 1.5f);
}

void AcidLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b,
    bool highlighted, bool)
{
    // BPM inc/dec buttons — render + and - with a larger, bolder font
    const auto txt = b.getButtonText();
    if (txt == "+" || txt == "-")
    {
        g.setColour(AcidColors::TextOnBtn);
        g.setFont(juce::Font("Arial", 18.f, juce::Font::bold));
        g.drawFittedText(txt, b.getLocalBounds(), juce::Justification::centred, 1);
        return;
    }

    const bool on = b.getToggleState();

    juce::Colour col;
    if (b.getProperties().contains("mlState"))
    {
        const int mlState = (int)b.getProperties()["mlState"];
        switch (mlState)
        {
            case 4:  col = TextDark;              break;  // learning green — dark text on green
            case 2:  col = TextDark;              break;  // learning bright — dark text on amber
            case 1:  col = LedAmber;              break;  // learning dim — amber text
            case 3:  col = LedGreen;              break;  // mapped — green text
            default: col = TextOnBtn.withAlpha(0.5f); break;  // idle — dim text
        }
    }
    else if (b.getProperties().contains("note"))
        col = on ? TextDark : TextOnBtn.withAlpha(0.7f);   // dark on white strobe, light otherwise
    else if (b.getProperties().contains("accent"))
        col = on ? LedGreen : TextOnBtn.withAlpha(0.5f);
    else if (b.getProperties().contains("slide"))
        col = on ? LedBlue  : TextOnBtn.withAlpha(0.5f);
    else if (b.getProperties().contains("active"))
        col = on ? LedRed : TextOnBtn.withAlpha(0.55f);
    else if (b.getProperties().contains("play"))
        col = LedGreen;  // always green text
    else if (b.getProperties().contains("lockbtn"))
        col = on ? juce::Colours::white : TextOnBtn.withAlpha(0.6f);
    else if (b.getProperties().contains("distbtn"))
        col = juce::Colour(0xFF00AAFF);   // always blue — DIST ON/OFF only
    else if (b.getProperties().contains("dist"))
        col = on ? LedWhite : TextOnBtn.withAlpha(0.8f);
    else if (b.getProperties().contains("delaynote"))
        col = on ? LedGreen : TextOnBtn.withAlpha(0.75f);
    else
        col = b.findColour(juce::TextButton::textColourOffId);

    g.setColour(col);
    const float fs = (float)b.getProperties().getWithDefault("fontSize", 9.5f);
    g.setFont(juce::Font("Arial", fs, juce::Font::bold));
    g.drawText(b.getButtonText(), b.getLocalBounds(),
               juce::Justification::centred, false);
}

juce::Font AcidLookAndFeel::getLabelFont(juce::Label& l)
{
    // BPM slider internal textbox — parent slider has "bpmSlider" property
    if (auto* parent = l.getParentComponent())
        if (parent->getProperties().contains("bpmSlider"))
            return juce::Font("Arial", 15.f, juce::Font::bold);

    return juce::Font("Arial", 9.f, juce::Font::bold);
}

// ── SOLAR303Editor ───────────────────────────────────────────────────────────────

SOLAR303Editor::SOLAR303Editor(SOLAR303Processor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&laf);
    setSize(900, 518);
    setTransform(juce::AffineTransform::scale(uiScale));
    setWantsKeyboardFocus(true);

    auto& apvts = processor.getAPVTS();
    auto& seq   = processor.getSequencer();

    // ── Play button (toggle: first click = play, second click = stop) ─────────
    playButton.setClickingTogglesState(true);
    playButton.getProperties().set("play", true);
    playButton.onClick = [this]
    {
        auto& seq = processor.getSequencer();
        if (seq.playing.load())
        {
            seq.stop();
            playButton.setButtonText("PLAY");
        }
        else
        {
            seq.pendingReset.store(true);
            seq.playing.store(true);
            playButton.setButtonText("STOP");
        }
    };
    addAndMakeVisible(playButton);

    // ── Skin button ───────────────────────────────────────────────────────────
    skinButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    skinButton.setColour(juce::TextButton::textColourOffId, AcidColors::LedWhite);
    skinButton.onClick = [this]
    {
        juce::PopupMenu menu;
        const auto skins = SOLAR303Skins::All();
        for (int i = 0; i < (int)skins.size(); ++i)
            menu.addItem(i + 1, skins[i].name,
                         true,
                         skins[i].name == currentSkin.name);

        menu.addSeparator();
        menu.addItem(100, "100%", true, std::abs(uiScale - 1.25f)   < 0.01f);
        menu.addItem(101, "75%",  true, std::abs(uiScale - 0.9375f) < 0.01f);
        menu.addItem(102, "50%",  true, std::abs(uiScale - 0.625f)  < 0.01f);

        menu.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(&skinButton),
            [this](int result)
            {
                if (result >= 100)
                {
                    uiScale = result == 100 ? 1.25f : result == 101 ? 0.9375f : 0.625f;
                    setTransform(juce::AffineTransform::scale(hostScale * uiScale));
                }
                else if (result > 0)
                {
                    const auto skins = SOLAR303Skins::All();
                    applySkin(skins[result - 1]);
                }
            });
    };
    addAndMakeVisible(skinButton);

    // ── BPM ───────────────────────────────────────────────────────────────────
    bpmSlider.setWantsKeyboardFocus(false);
    bpmSlider.setMouseClickGrabsKeyboardFocus(false);
    bpmSlider.setSliderStyle(juce::Slider::IncDecButtons);
    bpmSlider.getProperties().set("bpmSlider", true);   // used by getLabelFont to enlarge BPM digits
    bpmSlider.setRange(60.0, 240.0, 1.0);
    // Read initial BPM from sequencer (may already be synced from host)
    bpmSlider.setValue(processor.getSequencer().bpm.load(), juce::dontSendNotification);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 56, 26);
    bpmSlider.setColour(juce::Slider::textBoxTextColourId,       AcidColors::LedGreen);
    bpmSlider.setColour(juce::Slider::textBoxBackgroundColourId, AcidColors::BtnOff);
    bpmSlider.setColour(juce::Slider::textBoxOutlineColourId,    KnobRim);
    bpmSlider.onValueChange = [this]
    {
        processor.getSequencer().bpm.store((float)bpmSlider.getValue());
    };
    addAndMakeVisible(bpmSlider);

    bpmLabel.setText("BPM", juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId, TextOnBtn);
    bpmLabel.setFont(juce::Font("Arial", 9.f, juce::Font::bold));
    addAndMakeVisible(bpmLabel);

    // ── Length (number of active steps) ──────────────────────────────────────
    lengthSlider.setWantsKeyboardFocus(false);
    lengthSlider.setMouseClickGrabsKeyboardFocus(false);
    lengthSlider.setSliderStyle(juce::Slider::IncDecButtons);
    lengthSlider.getProperties().set("bpmSlider", true);   // reuse same bold-digit style
    lengthSlider.setRange(1.0, 16.0, 1.0);
    lengthSlider.setValue(processor.getSequencer().numSteps.load(), juce::dontSendNotification);
    lengthSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 34, 26);
    lengthSlider.setColour(juce::Slider::textBoxTextColourId,       AcidColors::LedGreen);
    lengthSlider.setColour(juce::Slider::textBoxBackgroundColourId, AcidColors::BtnOff);
    lengthSlider.setColour(juce::Slider::textBoxOutlineColourId,    KnobRim);
    lengthSlider.onValueChange = [this]
    {
        processor.getSequencer().numSteps.store((int)lengthSlider.getValue());
    };
    addAndMakeVisible(lengthSlider);

    lengthLabel.setText("STEPS", juce::dontSendNotification);
    lengthLabel.setColour(juce::Label::textColourId, TextOnBtn);
    lengthLabel.setFont(juce::Font("Arial", 9.f, juce::Font::bold));
    addAndMakeVisible(lengthLabel);

    // ── Random buttons ────────────────────────────────────────────────────────
    randomButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    randomButton.setColour(juce::TextButton::textColourOffId, AcidColors::LedRed);
    randomButton.onClick = [this] { randomiseAll(); };
    addAndMakeVisible(randomButton);

    randomKnobButton.setButtonText("REND K");
    randomKnobButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    randomKnobButton.setColour(juce::TextButton::textColourOffId, AcidColors::LedBlue);
    randomKnobButton.onClick = [this] { randomiseKnobs(); };
    addAndMakeVisible(randomKnobButton);

    randomStepsButton.setButtonText("REND S");
    randomStepsButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    randomStepsButton.setColour(juce::TextButton::textColourOffId, AcidColors::LedGreen);
    randomStepsButton.onClick = [this] { randomiseSteps(); };
    addAndMakeVisible(randomStepsButton);

    // ── RND history navigation: ◄ Back / ► Forward ───────────────────────────
    rndBackButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    rndBackButton.setColour(juce::TextButton::textColourOffId, AcidColors::TextOnBtn);
    rndBackButton.setEnabled(false);
    rndBackButton.onClick = [this]
    {
        if (rndHistIdx > 0)
        {
            --rndHistIdx;
            applySnapshot(rndHistory[rndHistIdx]);
            updateRndNavButtons();
        }
    };
    addAndMakeVisible(rndBackButton);

    rndFwdButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    rndFwdButton.setColour(juce::TextButton::textColourOffId, AcidColors::TextOnBtn);
    rndFwdButton.setEnabled(false);
    rndFwdButton.onClick = [this]
    {
        if (rndHistIdx < (int)rndHistory.size() - 1)
        {
            ++rndHistIdx;
            applySnapshot(rndHistory[rndHistIdx]);
            updateRndNavButtons();
        }
    };
    addAndMakeVisible(rndFwdButton);

    // ── Reset knobs button ────────────────────────────────────────────────────
    resetKnobsButton.setButtonText("RST 0%");
    resetKnobsButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    resetKnobsButton.setColour(juce::TextButton::textColourOffId, AcidColors::TextOnBtn);
    resetKnobsButton.onClick = [this]()
    {
        auto setParam = [this](const char* id, float normVal)
        {
            if (auto* p = processor.getAPVTS().getParameter(id))
            { p->beginChangeGesture(); p->setValueNotifyingHost(normVal); p->endChangeGesture(); }
        };

        if (!resetKnobsTo50)
        {
            // RST 0% — all knobs to minimum (cutoff closed, all modulation off)
            setParam("cutoff",     0.0f);
            setParam("resonance",  0.0f);
            setParam("envmod",     0.0f);
            setParam("decay",      0.0f);
            setParam("accent",     0.0f);
            setParam("slide",      0.0f);
        }
        else
        {
            // RST 50% — everything at midpoint
            setParam("cutoff",     0.5f);
            setParam("resonance",  0.5f);
            setParam("envmod",     0.5f);
            setParam("decay",      0.5f);
            setParam("accent",     0.5f);
            setParam("slide",      0.5f);
        }

        resetKnobsTo50 = !resetKnobsTo50;
        resetKnobsButton.setButtonText(resetKnobsTo50 ? "RST 50%" : "RST 0%");
    };
    addAndMakeVisible(resetKnobsButton);

    // ── Reset Sequencer button ────────────────────────────────────────────────
    resetSeqButton.setButtonText("RST SEQ");
    resetSeqButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    resetSeqButton.setColour(juce::TextButton::textColourOffId, AcidColors::TextOnBtn);
    resetSeqButton.onClick = [this]()
    {
        auto& seq = processor.getSequencer();
        for (int i = 0; i < 16; ++i)
        {
            seq.steps[i].active = true;
            seq.steps[i].note   = 48;   // C3
            seq.steps[i].accent = false;
            seq.steps[i].slide  = false;
        }
        // Sync UI buttons
        for (int i = 0; i < 16; ++i)
        {
            activeButtons[i].setToggleState(true,  juce::dontSendNotification);
            accentButtons[i].setToggleState(false, juce::dontSendNotification);
            slideButtons [i].setToggleState(false, juce::dontSendNotification);
            updateNoteButton(i);
        }
    };
    addAndMakeVisible(resetSeqButton);

    // ── MIDI drag button (VST3) / Preset buttons (Standalone) ───────────────
    const bool isStandalone =
        (processor.wrapperType == juce::AudioProcessor::wrapperType_Standalone);

    dragMidiButton.setButtonText("DRAG MIDI");
    dragMidiButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    dragMidiButton.setColour(juce::TextButton::textColourOffId, AcidColors::LedGreen);
    dragMidiButton.getProperties().set("fontSize", 12.f);
    dragMidiButton.onDragStarted = [this]() { exportAndDragMidi(); };

    // Right-click → popup with Export / Import preset options
    dragMidiButton.onRightClick = [this]()
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Export Preset...");
        menu.addItem(2, "Import Preset...");
        menu.addSeparator();
        menu.addItem(3, "Show in File Explorer");

        menu.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(&dragMidiButton),
            [this](int result)
            {
                if (result == 3)
                {
                    auto presetsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                          .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303");
                    presetsDir.createDirectory();
                    presetsDir.revealToUser();
                    return;
                }
                if (result == 1)
                {
                    // ── Export ──────────────────────────────────────────────
                    fileChooser = std::make_unique<juce::FileChooser>(
                        "Export Preset",
                        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                            .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303"),
                        "*.vstpreset");

                    fileChooser->launchAsync(
                        juce::FileBrowserComponent::saveMode |
                        juce::FileBrowserComponent::canSelectFiles,
                        [this](const juce::FileChooser& fc)
                        {
                            auto f = fc.getResult();
                            if (f == juce::File{}) return;
                            f = f.withFileExtension("vstpreset");
                            processor.saveVstPreset(f);
                            processor.scanUserPresets();
                            processor.updateHostDisplay();
                        });
                }
                else if (result == 2)
                {
                    // ── Import ──────────────────────────────────────────────
                    fileChooser = std::make_unique<juce::FileChooser>(
                        "Import Preset",
                        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                            .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303"),
                        "*.solar;*.zb303;*.vstpreset;*.tb303");

                    fileChooser->launchAsync(
                        juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles,
                        [this](const juce::FileChooser& fc)
                        {
                            auto f = fc.getResult();
                            if (f == juce::File{} || !f.existsAsFile()) return;
                            juce::MemoryBlock block;
                            f.loadFileAsData(block);
                            processor.setStateInformation(block.getData(), (int)block.getSize());
                            refreshStepGrid();
                        });
                }
            });
    };

    addAndMakeVisible(dragMidiButton);
    dragMidiButton.setVisible(!isStandalone);   // must be AFTER addAndMakeVisible

    // ── Preset Save / Load — Standalone only ─────────────────────────────────
    auto stylePresetBtn = [this](juce::TextButton& btn, juce::Colour textCol)
    {
        btn.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
        btn.setColour(juce::TextButton::buttonOnColourId, AcidColors::BtnHi);
        btn.setColour(juce::TextButton::textColourOffId, textCol);
        btn.setColour(juce::TextButton::textColourOnId,  textCol);
    };
    stylePresetBtn(savePresetButton, AcidColors::LedAmber);
    stylePresetBtn(loadPresetButton, AcidColors::LedGreen);

    savePresetButton.onClick = [this]()
    {
        auto presetsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                              .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303");
        presetsDir.createDirectory();   // creates folder if it doesn't exist yet

        fileChooser = std::make_unique<juce::FileChooser>(
            "Save Preset", presetsDir, "*.vstpreset");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File{}) return;
                f = f.withFileExtension("vstpreset");
                processor.saveVstPreset(f);
            });
    };

    loadPresetButton.onClick = [this]()
    {
        auto presetsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                              .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303");
        presetsDir.createDirectory();

        fileChooser = std::make_unique<juce::FileChooser>(
            "Load Preset", presetsDir, "*.solar;*.zb303;*.vstpreset;*.tb303");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File{} || !f.existsAsFile()) return;

                juce::MemoryBlock block;
                f.loadFileAsData(block);

                const auto* raw = static_cast<const char*>(block.getData());
                int sz = (int)block.getSize();

                // VST3 .vstpreset files have a Steinberg header.
                // The JUCE state chunk starts after "Comp" marker.
                // Search for the JUCE binary XML signature (starts with "VC2!")
                int offset = -1;
                for (int i = 0; i < sz - 4; ++i)
                {
                    if (raw[i] == 'V' && raw[i+1] == 'C' && raw[i+2] == '2' && raw[i+3] == '!')
                    {
                        offset = i;
                        break;
                    }
                }

                if (offset >= 0)
                    processor.setStateInformation(raw + offset, sz - offset);
                else
                    processor.setStateInformation(block.getData(), sz);

                refreshStepGrid();
            });
    };

    // Right-click on SAVE or LOAD → "Show in File Explorer"
    auto showPresetsFolder = [this]()
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Show in File Explorer");
        menu.showMenuAsync(juce::PopupMenu::Options(),
            [this](int result)
            {
                if (result == 1)
                {
                    auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                   .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303");
                    dir.createDirectory();
                    dir.revealToUser();
                }
            });
    };
    savePresetButton.onRightClick = showPresetsFolder;
    loadPresetButton.onRightClick = showPresetsFolder;

    // EXPLORER button — opens preset folder in Windows Explorer
    stylePresetBtn(showExplorerButton, AcidColors::LedWhite);
    showExplorerButton.onClick = [this]()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                       .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303");
        dir.createDirectory();
        dir.revealToUser();
    };

    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(loadPresetButton);
    savePresetButton.setVisible(isStandalone);
    loadPresetButton.setVisible(isStandalone);
    showExplorerButton.setVisible(false);

    // ── Export / Import preset buttons — VST3 only ───────────────────────────
    stylePresetBtn(exportPresetButton, AcidColors::LedGreen);
    stylePresetBtn(importPresetButton, AcidColors::LedRed);

    exportPresetButton.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Export Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303"),
            "*.solar;*.zb303;*.vstpreset;*.tb303");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File{}) return;
                if (f.getFileExtension().isEmpty())
                    f = f.withFileExtension("zb303");
                juce::MemoryBlock block;
                processor.getStateInformation(block);
                f.replaceWithData(block.getData(), block.getSize());
            });
    };

    importPresetButton.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Import Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("VST3 Presets").getChildFile("SOLAR").getChildFile("SOLAR 303"),
            "*.solar;*.zb303;*.vstpreset;*.tb303");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File{} || !f.existsAsFile()) return;
                juce::MemoryBlock block;
                f.loadFileAsData(block);

                // Search for VC2! marker (same as standalone load path)
                const auto* raw = static_cast<const char*>(block.getData());
                int sz = (int)block.getSize();
                int offset = -1;
                for (int i = 0; i < sz - 4; ++i)
                    if (raw[i]=='V' && raw[i+1]=='C' && raw[i+2]=='2' && raw[i+3]=='!')
                        { offset = i; break; }

                if (offset >= 0)
                    processor.setStateInformation(raw + offset, sz - offset);
                else
                    processor.setStateInformation(block.getData(), sz);

                refreshStepGrid();
                processor.updateHostDisplay();
                repaint();
            });
    };

    addAndMakeVisible(exportPresetButton);
    addAndMakeVisible(importPresetButton);
    exportPresetButton.setVisible(!isStandalone);
    importPresetButton.setVisible(!isStandalone);

    // ── Scale Lock (Root + Scale selectors, above DRAG MIDI) ─────────────────
    rootCombo.addItem("C",   1);
    rootCombo.addItem("C#",  2);
    rootCombo.addItem("D",   3);
    rootCombo.addItem("D#",  4);
    rootCombo.addItem("E",   5);
    rootCombo.addItem("F",   6);
    rootCombo.addItem("F#",  7);
    rootCombo.addItem("G",   8);
    rootCombo.addItem("G#",  9);
    rootCombo.addItem("A",   10);
    rootCombo.addItem("A#",  11);
    rootCombo.addItem("B",   12);
    rootCombo.setSelectedId(1, juce::dontSendNotification);
    rootCombo.onChange = [this] { rndRootNote = rootCombo.getSelectedId() - 1; };
    addAndMakeVisible(rootCombo);

    scaleCombo.addItem("Major",         1);
    scaleCombo.addItem("Minor",         2);
    scaleCombo.addItem("Dorian",        3);
    scaleCombo.addItem("Phrygian",      4);
    scaleCombo.addItem("Lydian",        5);
    scaleCombo.addItem("Mixolydian",    6);
    scaleCombo.addItem("Minor Penta",   7);
    scaleCombo.addItem("Major Penta",   8);
    scaleCombo.addItem("Blues",         9);
    scaleCombo.addItem("Harm. Minor",   10);
    scaleCombo.addItem("Chromatic",     11);
    scaleCombo.setSelectedId(1, juce::dontSendNotification);
    scaleCombo.onChange = [this] { rndScaleIdx = scaleCombo.getSelectedId() - 1; };
    addAndMakeVisible(scaleCombo);

    scaleLockButton.setClickingTogglesState(true);
    scaleLockButton.getProperties().set("lockbtn", true);
    scaleLockButton.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
    scaleLockButton.setColour(juce::TextButton::textColourOffId, AcidColors::TextOnBtn.withAlpha(0.6f));
    scaleLockButton.onClick = [this] { scaleLockButton.repaint(); };
    addAndMakeVisible(scaleLockButton);

    auto setupScaleLabel = [this](juce::Label& lbl, const juce::String& text)
    {
        lbl.setText(text, juce::dontSendNotification);
        lbl.setJustificationType(juce::Justification::centredLeft);
        lbl.setColour(juce::Label::textColourId, AcidColors::TextOnBtn.withAlpha(0.7f));
        lbl.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
        addAndMakeVisible(lbl);
    };
    setupScaleLabel(lRootCombo,  "KEY");
    setupScaleLabel(lScaleCombo, "SCALE");
    setupScaleLabel(lScaleLock,  "LOCK");

    // ── Note Range buttons (LOW NOTE / HIGH NOTE) — above KEY/SCALE ──────────
    auto setupRangeLabel = [this](juce::Label& lbl, const juce::String& text)
    {
        lbl.setText(text, juce::dontSendNotification);
        lbl.setJustificationType(juce::Justification::centredLeft);
        lbl.setColour(juce::Label::textColourId, AcidColors::TextOnBtn.withAlpha(0.7f));
        lbl.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
        addAndMakeVisible(lbl);
    };
    setupRangeLabel(lRndLowNote,  "LOW NOTE");
    setupRangeLabel(lRndHighNote, "HIGH NOTE");

    auto setupRangeBtn = [this](juce::TextButton& btn)
    {
        btn.setColour(juce::TextButton::buttonColourId,  AcidColors::BtnOff);
        btn.setColour(juce::TextButton::textColourOffId, AcidColors::LedAmber);
        addAndMakeVisible(btn);
    };
    setupRangeBtn(rndLowNoteButton);
    setupRangeBtn(rndHighNoteButton);

    rndLowNoteButton .setButtonText(midiNoteName(rndMinNote));
    rndHighNoteButton.setButtonText(midiNoteName(rndMaxNote));

    rndLowNoteButton .onClick = [this] { showNoteRangePicker(true);  };
    rndHighNoteButton.onClick = [this] { showNoteRangePicker(false); };

    // ── Step grid ─────────────────────────────────────────────────────────────
    for (int i = 0; i < 16; ++i)
    {
        // Note button
        noteButtons[i].getProperties().set("note", true);
        noteButtons[i].stepIndex = i;
        // Scroll wheel / right-click: ±1 semitone — wraps infinitely C1↔C6
        noteButtons[i].onNoteChange = [this](int step, int delta)
        {
            auto& s = processor.getSequencer().steps[step];
            // 61 notes in range [24..84]; wrap with modular arithmetic
            s.note = ((s.note - 24 + delta) % 61 + 61) % 61 + 24;
            updateNoteButton(step);
        };
        // Left-click popup selection
        noteButtons[i].onNotePicked = [this](int step, int note)
        {
            auto& s = processor.getSequencer().steps[step];
            s.note = juce::jlimit(24, 84, note);
            updateNoteButton(step);
        };

        // Press → noteOn  |  Release → noteOff
        // Sounds exactly as the step plays in the sequencer:
        //   accent → high velocity (>0.7) to trigger accent envelope
        //   slide  → portamento glide arriving at the note (from 5 semitones below)
        noteButtons[i].onPreviewStart = [this](int step)
        {
            const auto& s = processor.getSequencer().steps[step];
            const float vel = s.accent ? 1.0f : 0.5f;
            processor.previewNoteOn(s.note, vel, s.slide);
        };
        noteButtons[i].onPreviewStop = [this](int step)
        {
            const int note = processor.getSequencer().steps[step].note;
            processor.previewNoteOff(note);
        };

        addAndMakeVisible(noteButtons[i]);

        // Accent button
        accentButtons[i].setClickingTogglesState(true);
        accentButtons[i].setButtonText("ACC");
        accentButtons[i].getProperties().set("accent", true);
        accentButtons[i].onClick = [this, i]
        {
            processor.getSequencer().steps[i].accent = accentButtons[i].getToggleState();
        };
        addAndMakeVisible(accentButtons[i]);

        // Slide button
        slideButtons[i].setClickingTogglesState(true);
        slideButtons[i].setButtonText("SLD");
        slideButtons[i].getProperties().set("slide", true);
        slideButtons[i].onClick = [this, i]
        {
            processor.getSequencer().steps[i].slide = slideButtons[i].getToggleState();
        };
        addAndMakeVisible(slideButtons[i]);

        // Active button
        activeButtons[i].setClickingTogglesState(true);
        activeButtons[i].setButtonText(juce::String(i + 1));
        activeButtons[i].getProperties().set("active", true);
        activeButtons[i].onClick = [this, i]
        {
            processor.getSequencer().steps[i].active = activeButtons[i].getToggleState();
        };
        addAndMakeVisible(activeButtons[i]);
    }

    // Reflect whatever sequencer state exists (saved state restored by
    // setStateInformation, or factory defaults on first launch).
    // Do NOT call loadPreset() here — that would overwrite restored state.
    refreshStepGrid();

    // ── Synth knobs ───────────────────────────────────────────────────────────
    setupKnob(sCutoff,    lCutoff,    "CUTOFF");
    setupKnob(sResonance, lResonance, "RES");
    setupKnob(sEnvMod,    lEnvMod,    "ENV MOD");
    setupKnob(sDecay,     lDecay,     "DECAY");
    setupKnob(sAccent,    lAccent,    "ACCENT");
    setupKnob(sSlide,     lSlide,     "SLIDE");
    setupKnob(sTuning,    lTuning,    "TUNE");

    // Waveform
    waveCombo.addItem("Sawtooth", 1);
    waveCombo.addItem("Square",   2);
    waveCombo.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(waveCombo);

    lWave.setText("WAVEFORM", juce::dontSendNotification);
    lWave.setJustificationType(juce::Justification::centred);
    lWave.setColour(juce::Label::textColourId, LedWhite);
    lWave.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(lWave);

    // ── Distortion controls ───────────────────────────────────────────────────
    distOnButton.setClickingTogglesState(true);
    distOnButton.getProperties().set("distbtn", true);   // blue text, red bg when on
    distOnButton.onClick = [this]
    {
        distOnButton.setButtonText(
            distOnButton.getToggleState() ? "DIST ON" : "DIST OFF");
    };
    addAndMakeVisible(distOnButton);

    // Distortion type selector
    distTypeCombo.addItem("TANH",  1);   // smooth/warm
    distTypeCombo.addItem("HARD",  2);   // aggressive square clip
    distTypeCombo.addItem("FUZZ",  3);   // asymmetric germanium fuzz
    distTypeCombo.addItem("FOLD",  4);   // wavefolder — the 303 SCREAM
    distTypeCombo.addItem("TUBE",  5);   // asymmetric tube saturation
    distTypeCombo.addItem("CRUSH", 6);   // bit crusher — lo-fi digital
    distTypeCombo.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(distTypeCombo);

    lDistType.setText("TYPE", juce::dontSendNotification);
    lDistType.setJustificationType(juce::Justification::centredLeft);
    lDistType.setColour(juce::Label::textColourId, juce::Colour(0xFF00CC20));
    lDistType.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(lDistType);

    setupKnob(sDistAmount, lDistAmount, "AMOUNT");
    lDistAmount.setColour(juce::Label::textColourId, juce::Colour(0xFF00CC20));
    setupKnob(sDistVol,    lDistVol,    "DIST VOL");
    lDistVol   .setColour(juce::Label::textColourId, juce::Colour(0xFF00CC20));

    distTypeCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFF00CC20));

    // ── Reverb controls ───────────────────────────────────────────────────────
    revOnButton.setClickingTogglesState(true);
    revOnButton.getProperties().set("dist", true);
    revOnButton.onClick = [this]
    {
        revOnButton.setButtonText(
            revOnButton.getToggleState() ? "REV ON" : "REV OFF");
    };
    addAndMakeVisible(revOnButton);
    lRevOn.setText("ON / OFF", juce::dontSendNotification);
    lRevOn.setJustificationType(juce::Justification::centred);
    lRevOn.setColour(juce::Label::textColourId, TextOnBtn);
    lRevOn.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(lRevOn);
    setupKnob(sRevSize, lRevSize, "ROOM");
    setupKnob(sRevDamp, lRevDamp, "DAMP");
    setupKnob(sRevMix,  lRevMix,  "MIX");
    // FX knob labels need light text (dark background box)
    for (auto* l : { &lRevSize, &lRevDamp, &lRevMix })
        l->setColour(juce::Label::textColourId, TextOnBtn);

    // ── Delay controls ────────────────────────────────────────────────────────
    delayOnButton.setClickingTogglesState(true);
    delayOnButton.getProperties().set("dist", true);
    delayOnButton.onClick = [this]
    {
        const bool on = delayOnButton.getToggleState();
        delayOnButton.setButtonText(on ? "DLY ON" : "DLY OFF");

        // Auto-set 1/8 when turning ON (beatMult=0.5, index 0)
        if (on)
        {
            for (int j = 0; j < 12; ++j)
                delayNoteButtons[j].setToggleState(j == 0, juce::dontSendNotification);
            const float bpm  = processor.getSequencer().bpm.load();
            const float secs = juce::jlimit(0.02f, 2.0f, 0.5f * (60.0f / bpm));
            sDelayTime.setValue(secs, juce::sendNotification);
        }
    };
    addAndMakeVisible(delayOnButton);
    lDelayOn.setText("ON / OFF", juce::dontSendNotification);
    lDelayOn.setJustificationType(juce::Justification::centred);
    lDelayOn.setColour(juce::Label::textColourId, TextOnBtn);
    lDelayOn.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(lDelayOn);
    setupKnob(sDelayTime, lDelayTime, "TIME");
    setupKnob(sDelayFeed, lDelayFeed, "FEED");
    setupKnob(sDelayMix,  lDelayMix,  "MIX");
    // FX knob labels — light text
    for (auto* l : { &lDelayTime, &lDelayFeed, &lDelayMix })
        l->setColour(juce::Label::textColourId, TextOnBtn);

    // ── Delay note-division presets ───────────────────────────────────────────
    // 12 buttons: 3 rows (Straight / Triplet / Dotted) × 4 columns (1/8 1/4 1/2 4/4)
    // Click sets sDelayTime to the BPM-synced value for that note division.
    struct DNP { const char* label; float beatMult; };
    static const DNP notePresets[12] = {
        // ── Straight ─────────────────────────────────────────────────────────
        {"1/8",  0.5f},         {"1/4",  1.0f},
        {"1/2",  2.0f},         {"4/4",  4.0f},
        // ── Triplet (×2/3) ───────────────────────────────────────────────────
        {"1/8T", 1.0f/3.0f},   {"1/4T", 2.0f/3.0f},
        {"1/2T", 4.0f/3.0f},   {"4/4T", 8.0f/3.0f},
        // ── Dotted (×3/2) ────────────────────────────────────────────────────
        {"1/8D", 0.75f},        {"1/4D", 1.5f},
        {"1/2D", 3.0f},         {"4/4D", 6.0f},
    };
    for (int i = 0; i < 12; ++i)
    {
        delayNoteButtons[i].setButtonText(notePresets[i].label);
        delayNoteButtons[i].getProperties().set("delaynote", true);
        delayNoteButtons[i].getProperties().set("fontSize", 11.0f);
        const float mult = notePresets[i].beatMult;
        delayNoteButtons[i].onClick = [this, i, mult]()
        {
            // Exclusive highlight: turn off all, light up the selected one
            for (int j = 0; j < 12; ++j)
                delayNoteButtons[j].setToggleState(j == i, juce::dontSendNotification);

            const float bpm     = processor.getSequencer().bpm.load();
            const float secs    = mult * (60.0f / bpm);
            const float clamped = juce::jlimit(0.02f, 2.0f, secs);
            sDelayTime.setValue(clamped, juce::sendNotification);
        };
        addAndMakeVisible(delayNoteButtons[i]);
    }

    // Default: 1/8 button lit on startup
    delayNoteButtons[0].setToggleState(true, juce::dontSendNotification);

    // ── MIDI Learn buttons ─────────────────────────────────────────────────────
    setupMidiLearnButtons();

    // Restore ML keyboard keys from processor state
    for (int i = 0; i < 6; ++i) { mlKeyUp[i] = processor.mlKeyUp[i]; mlKeyDown[i] = processor.mlKeyDown[i]; }

    // ── Master Volume ─────────────────────────────────────────────────────────
    setupKnob(sMasterVol, lMasterVol, "MASTER VOL");
    lMasterVol.setColour(juce::Label::textColourId, TextOnBtn);


    // APVTS attachments
    aCutoff     = std::make_unique<SliderAtt>(apvts, "cutoff",      sCutoff);
    aResonance  = std::make_unique<SliderAtt>(apvts, "resonance",   sResonance);
    aEnvMod     = std::make_unique<SliderAtt>(apvts, "envmod",      sEnvMod);
    aDecay      = std::make_unique<SliderAtt>(apvts, "decay",       sDecay);
    aAccent     = std::make_unique<SliderAtt>(apvts, "accent",      sAccent);
    aSlide      = std::make_unique<SliderAtt>(apvts, "slide",       sSlide);
    aTuning     = std::make_unique<SliderAtt>(apvts, "tuning",      sTuning);
    aWave       = std::make_unique<ComboAtt> (apvts, "waveform",    waveCombo);
    aMasterVol  = std::make_unique<SliderAtt>(apvts, "mastervol",   sMasterVol);
    aRevOn      = std::make_unique<BoolAtt>  (apvts, "revon",       revOnButton);
    aRevSize    = std::make_unique<SliderAtt>(apvts, "revsize",     sRevSize);
    aRevDamp    = std::make_unique<SliderAtt>(apvts, "revdamp",     sRevDamp);
    aRevMix     = std::make_unique<SliderAtt>(apvts, "revmix",      sRevMix);
    aDelayOn    = std::make_unique<BoolAtt>  (apvts, "delayon",     delayOnButton);
    aDelayTime  = std::make_unique<SliderAtt>(apvts, "delaytime",   sDelayTime);
    aDelayFeed  = std::make_unique<SliderAtt>(apvts, "delayfeed",   sDelayFeed);
    aDelayMix   = std::make_unique<SliderAtt>(apvts, "delaymix",    sDelayMix);
    aDistOn     = std::make_unique<BoolAtt>  (apvts, "diston",      distOnButton);
    aDistType   = std::make_unique<ComboAtt> (apvts, "disttype",    distTypeCombo);
    aDistAmount = std::make_unique<SliderAtt>(apvts, "distamount",  sDistAmount);
    aDistVol    = std::make_unique<SliderAtt>(apvts, "distvol",     sDistVol);

    // ── Initial button text sync (APVTS sets toggle state but not text) ──────
    revOnButton  .setButtonText(revOnButton  .getToggleState() ? "REV ON"  : "REV OFF");
    delayOnButton.setButtonText(delayOnButton.getToggleState() ? "DLY ON"  : "DLY OFF");
    distOnButton .setButtonText(distOnButton .getToggleState() ? "DIST ON" : "DIST OFF");

    startTimerHz(20);

    // ── License check — Standalone is always free, VST3 requires activation ──
    if (! isStandalone && ! LicenseManager::isActivated())
    {
        activationScreen = std::make_unique<ActivationScreen>();
        activationScreen->onActivated = [this]()
        {
            activationScreen.reset();
            repaint();
        };
        addAndMakeVisible (*activationScreen);
        activationScreen->setBounds (getLocalBounds());
    }
}

// ── ML buttons setup — UP + DOWN per knob ───────────────────────────────────

void SOLAR303Editor::setupMidiLearnButtons()
{
    for (int i = 0; i < 6; ++i)
    {
        // ── UP button ────────────────────────────────────────────────────────
        mlUpButtons[i].setButtonText("↑");
        mlUpButtons[i].setClickingTogglesState(false);
        mlUpButtons[i].getProperties().set("mlState", 0);
        mlUpButtons[i].getProperties().set("fontSize", 9.f);

        mlUpButtons[i].onClick = [this, i]()
        {
            if (mlLearningKnob == i && mlLearningIsUp)
            {
                // Cancel
                mlLearningKnob = -1;
                processor.stopMidiLearn();
            }
            else
            {
                // Start learning UP
                mlLearningKnob = i;
                mlLearningIsUp = true;
                processor.startMidiLearn(i);
            }
        };

        // Right-click → clear this knob's CC + key assignment
        mlUpButtons[i].onRightClick = [this, i]()
        {
            processor.clearMidiLearn(i);
            mlKeyUp[i] = 0; processor.mlKeyUp[i] = 0;
            if (mlLearningKnob == i) { mlLearningKnob = -1; processor.stopMidiLearn(); }
        };

        addAndMakeVisible(mlUpButtons[i]);

        // ── DOWN button ──────────────────────────────────────────────────────
        mlDownButtons[i].setButtonText("↓");
        mlDownButtons[i].setClickingTogglesState(false);
        mlDownButtons[i].getProperties().set("mlState", 0);
        mlDownButtons[i].getProperties().set("fontSize", 9.f);

        mlDownButtons[i].onClick = [this, i]()
        {
            if (mlLearningKnob == i && !mlLearningIsUp)
            {
                // Cancel
                mlLearningKnob = -1;
                processor.stopMidiLearn();
            }
            else
            {
                // Start learning DOWN
                mlLearningKnob = i;
                mlLearningIsUp = false;
                processor.startMidiLearn(i);
            }
        };

        // Right-click → clear this knob's CC + key assignment
        mlDownButtons[i].onRightClick = [this, i]()
        {
            processor.clearMidiLearn(i);
            mlKeyDown[i] = 0; processor.mlKeyDown[i] = 0;
            if (mlLearningKnob == i) { mlLearningKnob = -1; processor.stopMidiLearn(); }
        };

        addAndMakeVisible(mlDownButtons[i]);
    }
}

juce::Slider* SOLAR303Editor::getSliderForMl(int index)
{
    switch (index)
    {
        case 0: return &sCutoff;
        case 1: return &sResonance;
        case 2: return &sEnvMod;
        case 3: return &sDecay;
        case 4: return &sAccent;
        case 5: return &sSlide;
        default: return nullptr;
    }
}

juce::String SOLAR303Editor::keyCodeName(int kc)
{
    if (kc >= 'A' && kc <= 'Z') return juce::String::charToString((juce::juce_wchar)kc);
    if (kc >= '0' && kc <= '9') return juce::String::charToString((juce::juce_wchar)kc);
    if (kc == juce::KeyPress::upKey)    return "UP";
    if (kc == juce::KeyPress::downKey)  return "DN";
    if (kc == juce::KeyPress::leftKey)  return "LT";
    if (kc == juce::KeyPress::rightKey) return "RT";
    if (kc == juce::KeyPress::spaceKey) return "SPC";
    return "#" + juce::String(kc);
}

bool SOLAR303Editor::keyPressed(const juce::KeyPress& key)
{
    const int kc = key.getKeyCode();

    // ── Learning mode ────────────────────────────────────────────────────────
    if (mlLearningKnob >= 0 && mlLearningKnob < 6)
    {
        // Clear any other knob that has this key
        for (int i = 0; i < 6; ++i) { if (mlKeyUp[i]   == kc) mlKeyUp[i]   = 0; }
        for (int i = 0; i < 6; ++i) { if (mlKeyDown[i]  == kc) mlKeyDown[i] = 0; }

        if (mlLearningIsUp)
            mlKeyUp  [mlLearningKnob] = kc;
        else
            mlKeyDown[mlLearningKnob] = kc;

        // Save immediately to processor so state persists
        for (int i = 0; i < 6; ++i) { processor.mlKeyUp[i] = mlKeyUp[i]; processor.mlKeyDown[i] = mlKeyDown[i]; }

        mlLearningKnob = -1;   // done
        processor.stopMidiLearn();
        return true;
    }

    // ── Normal mode: UP or DOWN key controls its knob ────────────────────────
    for (int i = 0; i < 6; ++i)
    {
        auto* slider = getSliderForMl(i);
        if (!slider) continue;

        const double range = slider->getMaximum() - slider->getMinimum();
        const double step  = range * 0.01;

        if (mlKeyUp[i] != 0 && mlKeyUp[i] == kc)
        {
            slider->setValue(slider->getValue() + step, juce::sendNotification);
            return true;
        }
        if (mlKeyDown[i] != 0 && mlKeyDown[i] == kc)
        {
            slider->setValue(slider->getValue() - step, juce::sendNotification);
            return true;
        }
    }

    return false;
}

SOLAR303Editor::~SOLAR303Editor()
{
    if (auto* top = getTopLevelComponent())
        top->removeKeyListener(this);
    stopTimer();
    setLookAndFeel(nullptr);
}

// ── Host scale factor (DPI) — combine with our custom uiScale ────────────────

void SOLAR303Editor::setScaleFactor(float newScale)
{
    hostScale = newScale;
    setTransform(juce::AffineTransform::scale(hostScale * uiScale));
}

// ── Timer (step highlight) ────────────────────────────────────────────────────

void SOLAR303Editor::timerCallback()
{
    // Register key listener on top-level component (once, when available)
    static bool keyListenerAdded = false;
    if (!keyListenerAdded)
    {
        if (auto* top = getTopLevelComponent())
        {
            top->addKeyListener(this);
            keyListenerAdded = true;
        }
    }


    // Keep PLAY button text in sync while sequencer is running
    const bool isPlaying = processor.getSequencer().playing.load();
    playButton.setToggleState(isPlaying, juce::dontSendNotification);
    playButton.setButtonText(isPlaying ? "STOP" : "PLAY");

    // ── Sync BPM slider with host (DAW) tempo ────────────────────────────────
    const float hostBpm = processor.getSequencer().bpm.load();
    if (std::abs(hostBpm - (float)bpmSlider.getValue()) > 0.5f)
        bpmSlider.setValue(hostBpm, juce::dontSendNotification);

    // ── ML button visual feedback ────────────────────────────────────────────
    static int blinkCounter = 0;
    blinkCounter++;
    const bool blinkOn = (blinkCounter % 10) < 5;

    for (int i = 0; i < 6; ++i)
    {
        // If MIDI CC was captured → end learning automatically
        const int cc = processor.midiCCMap[i].load();
        if (mlLearningKnob == i && cc >= 0 && processor.midiLearnActiveParam.load() < 0)
            mlLearningKnob = -1;

        // ── UP button ────────────────────────────────────────────────────────
        if (mlLearningKnob == i && mlLearningIsUp)
        {
            mlUpButtons[i].getProperties().set("mlState", blinkOn ? 4 : 0);
            mlUpButtons[i].setButtonText(blinkOn ? "?" : "↑");
        }
        else if (mlKeyUp[i] != 0)
        {
            mlUpButtons[i].getProperties().set("mlState", 3);
            mlUpButtons[i].setButtonText(keyCodeName(mlKeyUp[i]));
        }
        else if (cc >= 0)
        {
            mlUpButtons[i].getProperties().set("mlState", 3);
            mlUpButtons[i].setButtonText("CC" + juce::String(cc));
        }
        else
        {
            mlUpButtons[i].getProperties().set("mlState", 0);
            mlUpButtons[i].setButtonText("↑");
        }
        mlUpButtons[i].repaint();

        // ── DOWN button ──────────────────────────────────────────────────────
        if (mlLearningKnob == i && !mlLearningIsUp)
        {
            mlDownButtons[i].getProperties().set("mlState", blinkOn ? 4 : 0);
            mlDownButtons[i].setButtonText(blinkOn ? "?" : "↓");
        }
        else if (mlKeyDown[i] != 0)
        {
            mlDownButtons[i].getProperties().set("mlState", 3);
            mlDownButtons[i].setButtonText(keyCodeName(mlKeyDown[i]));
        }
        else if (cc >= 0)
        {
            // MIDI CC is bidirectional — same CC controls both ↑ and ↓
            mlDownButtons[i].getProperties().set("mlState", 3);
            mlDownButtons[i].setButtonText("CC" + juce::String(cc));
        }
        else
        {
            mlDownButtons[i].getProperties().set("mlState", 0);
            mlDownButtons[i].setButtonText("↓");
        }
        mlDownButtons[i].repaint();
    }

    // ── Step highlight ────────────────────────────────────────────────────────
    const int step = processor.getSequencer().currentStep.load();
    if (step == displayedStep) return;

    // Clear previous highlight
    if (displayedStep >= 0 && displayedStep < 16)
        noteButtons[displayedStep].setToggleState(false, juce::dontSendNotification);

    // Set new highlight
    if (step >= 0 && step < 16)
        noteButtons[step].setToggleState(true, juce::dontSendNotification);

    displayedStep = step;
}

// ── Skin ──────────────────────────────────────────────────────────────────────

void SOLAR303Editor::applySkin(const SOLAR303Skin& skin)
{
    currentSkin = skin;

    // Update component colours that can't be changed via paint()
    bpmLabel.setColour(juce::Label::textColourId, skin.chassisText);
    lengthLabel.setColour(juce::Label::textColourId, skin.chassisText);

    // Refresh all knob labels (they inherit from LookAndFeel but are set per-label)
    for (auto* lbl : { &lCutoff, &lResonance, &lEnvMod, &lDecay,
                       &lAccent, &lSlide, &lTuning, &lMasterVol,
                       &lRevSize, &lRevDamp, &lRevMix,
                       &lDelayTime, &lDelayFeed, &lDelayMix,
                       &lDistAmount, &lDistVol, &lDistType, &lWave,
                       &lRevOn, &lDelayOn })
        lbl->setColour(juce::Label::textColourId, skin.chassisText);

    repaint();
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void SOLAR303Editor::paint(juce::Graphics& g)
{
    // ── Chassis — gradient via image (workaround for renderer issues) ─────────
    {
        const int w = getWidth();
        const int h = getHeight();
        juce::Image bg(juce::Image::RGB, w, h, false);
        {
            juce::Graphics bg_g(bg);
            bg_g.setGradientFill(juce::ColourGradient(
                currentSkin.gradientTop,    0.f, 0.f,
                currentSkin.gradientBottom, 0.f, (float)h, false));
            bg_g.fillAll();
        }
        g.drawImageAt(bg, 0, 0);
    }

    // Top edge highlight
    g.setColour(juce::Colour(0x30ffffff));
    g.fillRect(0, 0, getWidth(), 2);

    // ── Header bar — reuses same gradient image already drawn ─────────────────
    // (header already covered by the full-height image above)
    // Bottom edge of header — subtle shadow line
    g.setColour(juce::Colour(0x50000000));
    g.drawLine(0.f, 36.f, (float)getWidth(), 36.f, 1.5f);
    g.setColour(juce::Colour(0x20ffffff));
    g.drawLine(0.f, 37.f, (float)getWidth(), 37.f, 1.f);

    // ── Branding — SOLAR (orange) + 303 (cyan) in DSEG7 neon ─────────────────
    {
        g.setFont(dseg7(22.f));

        auto drawNeon = [&](const juce::String& txt, int tx, int ty, int tw, int th,
                            juce::Colour neon)
        {
            for (int dx = -3; dx <= 3; ++dx)
                for (int dy = -3; dy <= 3; ++dy)
                    if (dx != 0 || dy != 0)
                    {
                        const float dist = std::sqrt((float)(dx*dx + dy*dy));
                        g.setColour(neon.withAlpha(juce::jmax(0.f, 0.18f - dist * 0.04f)));
                        g.drawText(txt, tx + dx, ty + dy, tw, th, juce::Justification::centredLeft);
                    }
            g.setColour(neon.withAlpha(0.95f));
            g.drawText(txt, tx, ty, tw, th, juce::Justification::centredLeft);
            g.setColour(juce::Colours::white.withAlpha(0.45f));
            g.drawText(txt, tx, ty, tw, th, juce::Justification::centredLeft);
        };

        drawNeon("SOLAR", 50,  4,  82, 30, juce::Colour(0xFFFF6600));  // brand orange
        drawNeon("303",  130,  4,  65, 30, juce::Colour(0xFF00E5FF));  // neon cyan — adjacent
    }

    // "made by SOLAR vst" — below REVERB box, centred under it (x=87,w=230,centre=202)
    g.setFont(acakadut(14.f));
    g.setColour(juce::Colours::white.withAlpha(0.50f));
    g.drawText("made by", 714, 210, 88, 18, juce::Justification::centredLeft);

    g.setFont(easyRide(30.f));
    g.setColour(juce::Colours::white.withAlpha(0.50f));
    g.drawText("SOLAR", 762, 197, 135, 40, juce::Justification::centredLeft);

    g.setFont(acakadut(18.f));
    g.setColour(juce::Colours::white.withAlpha(0.46f));
    g.drawText("vst", 851, 214, 44, 22, juce::Justification::centredLeft);


    // ── Step grid section background ──────────────────────────────────────────
    // (gradient image from above already covers this area)

    // Separator line between grid and synth section
    g.setColour(juce::Colour(0x60000000));
    g.drawLine(0.f, 248.f, (float)getWidth(), 248.f, 1.5f);
    g.setColour(juce::Colour(0x20ffffff));
    g.drawLine(0.f, 249.5f, (float)getWidth(), 249.5f, 1.f);

    // ── Synth section background ───────────────────────────────────────────────
    // (gradient image from above already covers this area)

    // ── Step row labels ────────────────────────────────────────────────────────
    g.setColour(currentSkin.chassisText);
    g.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    g.drawText("NOTE",  2, 52,  36, 14, juce::Justification::centredRight);
    g.drawText("ACC",   2, 83,  36, 14, juce::Justification::centredRight);
    g.drawText("SLD",   2, 105, 36, 14, juce::Justification::centredRight);
    g.drawText("STEP",  2, 127, 36, 14, juce::Justification::centredRight);

    // ── FX separator ──────────────────────────────────────────────────────────
    g.setColour(juce::Colour(0x60000000));
    g.drawLine(0.f, 382.f, (float)getWidth(), 382.f, 1.5f);
    g.setColour(juce::Colour(0x20ffffff));
    g.drawLine(0.f, 383.5f, (float)getWidth(), 383.5f, 1.f);

    // ── Helper lambda: draw an FX box with red title strip ────────────────────
    auto drawFxBox = [&](int bx, int by, int bw, int bh, const char* title)
    {
        g.setColour(BtnOff);
        g.fillRoundedRectangle((float)bx, (float)by, (float)bw, (float)bh, 4.f);
        g.setColour(BtnRim);
        g.drawRoundedRectangle((float)bx, (float)by, (float)bw, (float)bh, 4.f, 1.5f);
        g.setGradientFill(juce::ColourGradient(
            RolandRed.brighter(0.35f), (float)bx, (float)by,
            RolandRed.darker  (0.45f), (float)bx, (float)(by + 16), false));
        g.fillRoundedRectangle((float)bx, (float)by, (float)bw, 16.f, 4.f);
        g.fillRect((float)bx, (float)(by + 8), (float)bw, 8.f);
        g.setColour(TextOnBtn);
        g.setFont(juce::Font("Arial", 8.5f, juce::Font::bold));
        g.drawText(title, bx, by, bw, 16, juce::Justification::centred);
    };

    drawFxBox( 87, 385, 230,  78, "REVERB");
    drawFxBox(325, 385, 230, 118, "DELAY");    // taller — room for note-div presets
    drawFxBox(563, 385, 250, 100, "DISTORTION");  // taller — 2 rows of buttons


    // Row labels for delay note presets
    g.setColour(TextOnBtn.withAlpha(0.55f));
    g.setFont(juce::Font("Arial", 7.5f, juce::Font::bold));
    g.drawText("SRT", 329, 385 + 68, 30, 12, juce::Justification::centredRight);
    g.drawText("T",   329, 385 + 82, 30, 12, juce::Justification::centredRight);
    g.drawText("D",   329, 385 + 96, 30, 12, juce::Justification::centredRight);
}

// ── Resized ───────────────────────────────────────────────────────────────────

void SOLAR303Editor::resized()
{
    constexpr int stepW  = 50;
    constexpr int startX = 50;

    // ── Header row ────────────────────────────────────────────────────────────
    // Button group — centred in 900 px, same y as DRAG MIDI button (y=191)
    playButton        .setBounds(433, 161,  60, 18);   // swapped — now on top row
    randomButton      .setBounds(503, 161,  44, 18);   // REND A
    randomKnobButton  .setBounds(551, 161,  44, 18);   // REND K
    randomStepsButton .setBounds(599, 161,  44, 18);   // REND S
    rndBackButton     .setBounds(647, 161,  22, 18);   // ◄
    rndFwdButton      .setBounds(673, 161,  22, 18);   // ►
    resetKnobsButton  .setBounds(613, 335, 90, 22);   // RST — centred with waveCombo at x=658

    // ── Note Range (LOW NOTE / HIGH NOTE) — above KEY/SCALE ─────────────────
    lRndLowNote      .setBounds(205, 148,  68, 12);
    lRndHighNote     .setBounds(279, 148,  76, 12);
    rndLowNoteButton .setBounds(205, 161,  68, 20);
    rndHighNoteButton.setBounds(279, 161,  76, 20);

    // ── Scale Lock controls ───────────────────────────────────────────────────
    lRootCombo  .setBounds(205, 187,  52, 12);
    lScaleCombo .setBounds(279, 187,  88, 12);
    lScaleLock  .setBounds(371, 187,  38, 12);
    rootCombo   .setBounds(205, 200,  52, 18);
    scaleCombo  .setBounds(279, 200,  88, 18);
    scaleLockButton.setBounds(371, 200,  38, 18);

    // VST3: EXPORT + IMPORT in header; DRAG MIDI in the empty strip below step grid
    dragMidiButton      .setBounds(50, 181, 120, 18);   // centred vertically between LOW NOTE and KEY rows
    exportPresetButton  .setBounds(489,  6,  55, 24);
    importPresetButton  .setBounds(548,  6,  55, 24);
    // Standalone: SAVE + LOAD + EXPLORER
    savePresetButton    .setBounds(471,  6,  55, 24);
    loadPresetButton    .setBounds(530,  6,  55, 24);
    showExplorerButton  .setBounds(589,  6,  75, 24);

    bpmLabel .setBounds(331, 10,  30, 16);
    bpmSlider.setBounds(359,  4, 104, 30);
    // STEPS — parallel (label + slider side by side), left edge aligned with PLAY button (x=265)
    lengthLabel .setBounds(399, 189, 30, 12);
    lengthSlider.setBounds(433, 200, 82, 20);
    resetSeqButton.setBounds(519, 200, 70, 20);   // RST SEQ — swapped to bottom row
    skinButton.setBounds(736, 6,  60, 24);

    // ── Step grid ─────────────────────────────────────────────────────────────
    for (int i = 0; i < 16; ++i)
    {
        const int x = startX + i * stepW;
        noteButtons  [i].setBounds(x, 40,  stepW - 2, 38);
        accentButtons[i].setBounds(x, 80,  stepW - 2, 20);
        slideButtons [i].setBounds(x, 102, stepW - 2, 20);
        activeButtons[i].setBounds(x, 124, stepW - 2, 20);
    }

    // ── Synth knobs ───────────────────────────────────────────────────────────
    constexpr int kY  = 252;
    constexpr int kW  = 72;
    constexpr int kH  = 70;
    constexpr int lH  = 13;

    constexpr int kX0 = 153;  // left margin — centres 6-knob group between x=0 and TUNE (x=735)

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int col)
    {
        s.setBounds(col * kW + kX0, kY,          kW - 4, kH);
        l.setBounds(col * kW + kX0, kY + kH + 2, kW - 4, lH);
    };

    placeKnob(sCutoff,    lCutoff,    0);
    placeKnob(sResonance, lResonance, 1);
    placeKnob(sEnvMod,    lEnvMod,    2);
    placeKnob(sDecay,     lDecay,     3);
    placeKnob(sAccent,    lAccent,    4);
    placeKnob(sSlide,     lSlide,     5);

    // ── ML UP/DOWN buttons — below each knob label ──────────────────────────
    constexpr int mlBtnW = 28;
    constexpr int mlBtnH = 15;
    constexpr int mlGap  = 2;
    constexpr int mlY    = kY + kH + lH + 4;
    for (int i = 0; i < 6; ++i)
    {
        const int cx = i * kW + kX0 + (kW - 4) / 2;
        mlDownButtons[i].setBounds(cx - mlBtnW - mlGap / 2, mlY, mlBtnW, mlBtnH);
        mlUpButtons  [i].setBounds(cx + mlGap / 2,           mlY, mlBtnW, mlBtnH);
    }

    // TUNE: moved next to Master Volume (see below)

    // Waveform — centred between SLIDE right edge (581) and TUNE left edge (735), centre=658
    lWave   .setBounds(623, kY + 10, 70, 13);
    waveCombo.setBounds(608, kY + 26, 100, 24);

    // ── FX section (y=385, two boxes: REVERB x=10 and DELAY x=248) ───────────
    constexpr int fxY  = 385;    // top of FX section (below separator at 382)
    constexpr int fkH  = 52;     // FX knob height
    constexpr int fkW  = 54;     // FX knob width
    constexpr int flH  = 12;     // FX label height

    // REVERB box (x=87, w=230)
    revOnButton.setBounds(91,   fxY + 18, 58, 20);
    lRevOn     .setBounds(91,   fxY + 40, 58, 11);
    sRevSize   .setBounds(155,  fxY +  6, fkW, fkH);
    lRevSize   .setBounds(155,  fxY +  6 + fkH - 2, fkW, flH);
    sRevDamp   .setBounds(213,  fxY +  6, fkW, fkH);
    lRevDamp   .setBounds(213,  fxY +  6 + fkH - 2, fkW, flH);
    sRevMix    .setBounds(267,  fxY +  6, 46, fkH);
    lRevMix    .setBounds(267,  fxY +  6 + fkH - 2, 46, flH);

    // DELAY box (x=325, w=230)
    delayOnButton.setBounds(329, fxY + 18, 58, 20);
    lDelayOn     .setBounds(329, fxY + 40, 58, 11);
    sDelayTime   .setBounds(393, fxY +  6, fkW, fkH);
    lDelayTime   .setBounds(393, fxY +  6 + fkH - 2, fkW, flH);
    sDelayFeed   .setBounds(451, fxY +  6, fkW, fkH);
    lDelayFeed   .setBounds(451, fxY +  6 + fkH - 2, fkW, flH);
    sDelayMix    .setBounds(507, fxY +  6, 44, fkH);
    lDelayMix    .setBounds(507, fxY +  6 + fkH - 2, 44, flH);

    // ── Delay note-division preset buttons (3 rows × 4 cols inside DELAY box) ──
    constexpr int dnX0 = 361;   // starts after the SRT/T/D labels (+39)
    constexpr int dnBW = 40;    // button width
    constexpr int dnBH = 16;    // button height
    constexpr int dnGX = 3;     // horizontal gap
    constexpr int dnGY = 2;     // vertical gap
    constexpr int dnY0 = fxY + 66;   // top of first row

    for (int i = 0; i < 12; ++i)
    {
        const int col = i % 4;
        const int row = i / 4;
        delayNoteButtons[i].setBounds(
            dnX0 + col * (dnBW + dnGX),
            dnY0 + row * (dnBH + dnGY),
            dnBW, dnBH);
    }

    // ── Distortion section (x=563, centred) ──────────────────────────────────
    {
        constexpr int dkW = 54;
        distOnButton  .setBounds(567, fxY + 20, 72, 20);
        lDistType     .setBounds(567, fxY + 44, 72, 11);
        distTypeCombo .setBounds(567, fxY + 56, 72, 20);
        sDistAmount   .setBounds(645, fxY +  8, dkW, 52);
        lDistAmount   .setBounds(645, fxY + 58, dkW, 12);
        sDistVol      .setBounds(703, fxY +  8, 52,  52);
        lDistVol      .setBounds(703, fxY + 58, 52,  12);
    }

    // ── TUNE + Master Volume — pair centred in area right of waveCombo (x=718–900) ──
    // knob width 66, gap 12 → total 144, side padding 15 → start x=735
    constexpr int pairY  = 283;
    constexpr int pairW  = 66;
    constexpr int pairH  = 66;
    constexpr int pairG  = 12;
    constexpr int pairX0 = 735;   // ((718+900) - (66+12+66)) / 2 + 718 ≈ 735
    sTuning   .setBounds(pairX0,          pairY,          pairW, pairH);
    lTuning   .setBounds(pairX0,          pairY + pairH + 2, pairW, lH);
    sMasterVol.setBounds(pairX0 + pairW + pairG, pairY,          pairW, pairH);
    lMasterVol.setBounds(pairX0 + pairW + pairG, pairY + pairH + 2, pairW, lH);

    if (activationScreen)
        activationScreen->setBounds (getLocalBounds());
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void SOLAR303Editor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 13);
    s.setWantsKeyboardFocus(false);
    s.setMouseClickGrabsKeyboardFocus(false);
    addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, TextOnBtn);
    l.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(l);
}

void SOLAR303Editor::randomiseAll()
{
    pushRndSnapshot();   // save state BEFORE randomising

    juce::Random rng;   // seeded from system clock automatically

    auto& seq  = processor.getSequencer();
    auto& apvts = processor.getAPVTS();

    // ── Sequencer steps ───────────────────────────────────────────────────────
    // Notes: minor pentatonic scale across 3 octaves (classic acid territory)
    // Root = C, offsets from C1 (MIDI 24): 0,3,5,7,10 per octave × 3 octaves
    static const int penta[] = { 0,3,5,7,10, 12,15,17,19,22, 24,27,29,31,34 };
    constexpr int numPenta    = (int)(sizeof(penta) / sizeof(penta[0]));

    for (int i = 0; i < 16; ++i)
    {
        auto& step = seq.steps[i];

        // ~75 % chance the step is active (ensures a dense-enough pattern)
        step.active  = (rng.nextFloat() < 0.75f);

        // Note: if Scale Lock is ON, pick from selected root+scale; else full random
        if (scaleLockButton.getToggleState())
        {
            static const int scaleNotes[][12] = {
                {0, 2, 4, 5, 7, 9, 11},          // Major
                {0, 2, 3, 5, 7, 8, 10},          // Minor
                {0, 2, 3, 5, 7, 9, 10},          // Dorian
                {0, 1, 3, 5, 7, 8, 10},          // Phrygian
                {0, 2, 4, 6, 7, 9, 11},          // Lydian
                {0, 2, 4, 5, 7, 9, 10},          // Mixolydian
                {0, 3, 5, 7, 10},                // Minor Penta
                {0, 2, 4, 7, 9},                 // Major Penta
                {0, 3, 5, 6, 7, 10},             // Blues
                {0, 2, 3, 5, 7, 8, 11},          // Harmonic Minor
                {0,1,2,3,4,5,6,7,8,9,10,11},     // Chromatic
            };
            static const int scaleNumNotes[] = {7,7,7,7,7,7,5,5,6,7,12};
            const int si = juce::jlimit(0, 10, rndScaleIdx);
            const int* intervals = scaleNotes[si];
            const int  numN      = scaleNumNotes[si];
            std::vector<int> pool;
            for (int midi = rndMinNote; midi <= rndMaxNote; ++midi)
            {
                int pc = ((midi - rndRootNote) % 12 + 12) % 12;
                for (int ii = 0; ii < numN; ++ii)
                    if (intervals[ii] == pc) { pool.push_back(midi); break; }
            }
            step.note = pool.empty() ? rndMinNote : pool[rng.nextInt((int)pool.size())];
        }
        else
        {
            // Free random — pick any note in the user-defined range
            step.note = rndMinNote + rng.nextInt(juce::jmax(1, rndMaxNote - rndMinNote + 1));
            step.note = juce::jlimit(rndMinNote, rndMaxNote, step.note);
        }

        // ~20 % accent — sparse so the pattern still breathes
        step.accent  = (rng.nextFloat() < 0.20f);

        // ~15 % slide — just enough for the classic acid glide
        step.slide   = (rng.nextFloat() < 0.15f);

        updateNoteButton(i);
        accentButtons[i].setToggleState(step.accent, juce::dontSendNotification);
        slideButtons [i].setToggleState(step.slide,  juce::dontSendNotification);
        activeButtons[i].setToggleState(step.active, juce::dontSendNotification);
    }

    // ── Synth parameters ──────────────────────────────────────────────────────
    // Each param gets a random normalised value (0-1), respecting its range/skew.
    // FX on/off and waveform are left untouched (too destructive to randomise).
    auto rndParam = [&](const juce::String& id)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(rng.nextFloat());
    };

    // Cutoff: minimum 30 Hz — normalised lower bound ≈ 0.215 on [20–4700, skew 0.25]
    if (auto* p = apvts.getParameter("cutoff"))
        p->setValueNotifyingHost(juce::jmax(0.215f, rng.nextFloat()));
    rndParam("resonance");
    rndParam("envmod");
    rndParam("decay");
    rndParam("accent");
    rndParam("slide");
    // TUNE is intentionally NOT randomised — stays at user's manual setting

    finaliseRndSnapshot();   // save post-random state → enables ► forward button
}

void SOLAR303Editor::randomiseKnobs()
{
    pushRndSnapshot();   // save state BEFORE randomising

    juce::Random rng;
    auto& apvts = processor.getAPVTS();

    auto rndParam = [&](const juce::String& id)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(rng.nextFloat());
    };

    // Cutoff: minimum 30 Hz — normalised lower bound ≈ 0.215 on [20–4700, skew 0.25]
    if (auto* p = apvts.getParameter("cutoff"))
        p->setValueNotifyingHost(juce::jmax(0.215f, rng.nextFloat()));
    rndParam("resonance");
    rndParam("envmod");
    rndParam("decay");
    rndParam("accent");
    rndParam("slide");
    // TUNE is intentionally NOT randomised — stays at user's manual setting
    // Steps and note buttons are NOT touched

    finaliseRndSnapshot();
}

void SOLAR303Editor::randomiseSteps()
{
    pushRndSnapshot();   // save state BEFORE randomising

    juce::Random rng;
    auto& seq = processor.getSequencer();

    // ── Sequencer steps only — all knobs stay exactly as set ─────────────────
    static const int penta[] = { 0,3,5,7,10, 12,15,17,19,22, 24,27,29,31,34 };
    constexpr int numPenta    = (int)(sizeof(penta) / sizeof(penta[0]));

    for (int i = 0; i < 16; ++i)
    {
        auto& step = seq.steps[i];

        step.active = (rng.nextFloat() < 0.75f);

        // Note: if Scale Lock is ON, pick from selected root+scale; else full random
        if (scaleLockButton.getToggleState())
        {
            static const int scaleNotes[][12] = {
                {0, 2, 4, 5, 7, 9, 11},          // Major
                {0, 2, 3, 5, 7, 8, 10},          // Minor
                {0, 2, 3, 5, 7, 9, 10},          // Dorian
                {0, 1, 3, 5, 7, 8, 10},          // Phrygian
                {0, 2, 4, 6, 7, 9, 11},          // Lydian
                {0, 2, 4, 5, 7, 9, 10},          // Mixolydian
                {0, 3, 5, 7, 10},                // Minor Penta
                {0, 2, 4, 7, 9},                 // Major Penta
                {0, 3, 5, 6, 7, 10},             // Blues
                {0, 2, 3, 5, 7, 8, 11},          // Harmonic Minor
                {0,1,2,3,4,5,6,7,8,9,10,11},     // Chromatic
            };
            static const int scaleNumNotes[] = {7,7,7,7,7,7,5,5,6,7,12};
            const int si = juce::jlimit(0, 10, rndScaleIdx);
            const int* intervals = scaleNotes[si];
            const int  numN      = scaleNumNotes[si];
            std::vector<int> pool;
            for (int midi = rndMinNote; midi <= rndMaxNote; ++midi)
            {
                int pc = ((midi - rndRootNote) % 12 + 12) % 12;
                for (int ii = 0; ii < numN; ++ii)
                    if (intervals[ii] == pc) { pool.push_back(midi); break; }
            }
            step.note = pool.empty() ? rndMinNote : pool[rng.nextInt((int)pool.size())];
        }
        else
        {
            // Free random — pick any note in the user-defined range
            step.note = rndMinNote + rng.nextInt(juce::jmax(1, rndMaxNote - rndMinNote + 1));
            step.note = juce::jlimit(rndMinNote, rndMaxNote, step.note);
        }

        step.accent = (rng.nextFloat() < 0.20f);
        step.slide  = (rng.nextFloat() < 0.15f);

        updateNoteButton(i);
        accentButtons[i].setToggleState(step.accent, juce::dontSendNotification);
        slideButtons [i].setToggleState(step.slide,  juce::dontSendNotification);
        activeButtons[i].setToggleState(step.active, juce::dontSendNotification);
    }
    // Knobs are intentionally NOT touched

    finaliseRndSnapshot();
}

void SOLAR303Editor::updateNoteButton(int step)
{
    const auto& s = processor.getSequencer().steps[step];
    noteButtons[step].setButtonText(
        juce::String(SOLAR303Sequencer::noteToString(s.note)));
}


juce::Slider* SOLAR303Editor::getSliderForParam(const juce::String& pid)
{
    if (pid == "cutoff")     return &sCutoff;
    if (pid == "resonance")  return &sResonance;
    if (pid == "envmod")     return &sEnvMod;
    if (pid == "decay")      return &sDecay;
    if (pid == "accent")     return &sAccent;
    if (pid == "slide")      return &sSlide;
    if (pid == "tuning")     return &sTuning;
    if (pid == "mastervol")  return &sMasterVol;
    if (pid == "revsize")    return &sRevSize;
    if (pid == "revdamp")    return &sRevDamp;
    if (pid == "revmix")     return &sRevMix;
    if (pid == "delaytime")  return &sDelayTime;
    if (pid == "delayfeed")  return &sDelayFeed;
    if (pid == "delaymix")   return &sDelayMix;
    if (pid == "distamount") return &sDistAmount;
    if (pid == "distvol")    return &sDistVol;
    return nullptr;
}

juce::Label* SOLAR303Editor::getLabelForParam(const juce::String& pid)
{
    if (pid == "cutoff")    return &lCutoff;
    if (pid == "resonance") return &lResonance;
    if (pid == "envmod")    return &lEnvMod;
    if (pid == "decay")     return &lDecay;
    if (pid == "accent")    return &lAccent;
    if (pid == "slide")     return &lSlide;
    if (pid == "tuning")    return &lTuning;
    if (pid == "mastervol") return &lMasterVol;
    if (pid == "revsize")   return &lRevSize;
    if (pid == "revdamp")   return &lRevDamp;
    if (pid == "revmix")    return &lRevMix;
    if (pid == "delaytime") return &lDelayTime;
    if (pid == "delayfeed") return &lDelayFeed;
    if (pid == "delaymix")  return &lDelayMix;
    if (pid == "distamount") return &lDistAmount;
    if (pid == "distvol")   return &lDistVol;
    return nullptr;
}


void SOLAR303Editor::refreshStepGrid()
{
    for (int i = 0; i < 16; ++i)
    {
        const auto& s = processor.getSequencer().steps[i];
        updateNoteButton(i);
        accentButtons[i].setToggleState(s.accent, juce::dontSendNotification);
        slideButtons [i].setToggleState(s.slide,  juce::dontSendNotification);
        activeButtons[i].setToggleState(s.active, juce::dontSendNotification);
    }

    // Sync BPM + Length sliders with the sequencer (restored from saved state)
    bpmSlider.setValue(processor.getSequencer().bpm.load(), juce::dontSendNotification);
    lengthSlider.setValue(processor.getSequencer().numSteps.load(), juce::dontSendNotification);
}

// ── Native Windows drag helper (CF_HDROP only — no extra formats that confuse Cubase) ──
#if JUCE_WINDOWS
namespace {

class HDropDataObject final : public IDataObject
{
public:
    explicit HDropDataObject (const juce::String& filePath)
    {
        auto* wPath     = filePath.toWideCharPointer();
        const size_t pathLen  = std::wcslen (wPath);
        const size_t memSize  = sizeof (DROPFILES) + (pathLen + 2) * sizeof (wchar_t);
        hDrop_ = ::GlobalAlloc (GHND, memSize);
        if (hDrop_)
        {
            auto* df   = static_cast<DROPFILES*> (::GlobalLock (hDrop_));
            df->pFiles = sizeof (DROPFILES);
            df->fWide  = TRUE;
            df->fNC    = FALSE;
            df->pt     = {};
            auto* dest = reinterpret_cast<wchar_t*> (df + 1);
            std::wmemcpy (dest, wPath, pathLen + 1);
            dest[pathLen + 1] = L'\0';   // double null-terminate
            ::GlobalUnlock (hDrop_);
        }
    }
    ~HDropDataObject() { if (hDrop_) ::GlobalFree (hDrop_); }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void** ppv) override
    {
        if (riid == IID_IUnknown || riid == IID_IDataObject)
            { *ppv = static_cast<IDataObject*> (this); AddRef(); return S_OK; }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef()  override { return (ULONG)::InterlockedIncrement (&ref_); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        LONG r = ::InterlockedDecrement (&ref_);
        if (r == 0) delete this;
        return (ULONG)r;
    }

    // IDataObject — only CF_HDROP is offered
    HRESULT STDMETHODCALLTYPE GetData (FORMATETC* pfe, STGMEDIUM* pstm) override
    {
        if (!hDrop_ || pfe->cfFormat != CF_HDROP || !(pfe->tymed & TYMED_HGLOBAL))
            return DV_E_FORMATETC;
        pstm->tymed          = TYMED_HGLOBAL;
        pstm->pUnkForRelease = nullptr;
        pstm->hGlobal        = ::OleDuplicateData (hDrop_, CF_HDROP, GHND);
        return pstm->hGlobal ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT STDMETHODCALLTYPE GetDataHere (FORMATETC*, STGMEDIUM*)          override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE QueryGetData (FORMATETC* pfe)                 override { return pfe->cfFormat == CF_HDROP ? S_OK : DV_E_FORMATETC; }
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc (FORMATETC*, FORMATETC* pOut) override { pOut->ptd = nullptr; return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetData (FORMATETC*, STGMEDIUM*, BOOL)        override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE EnumFormatEtc (DWORD dir, IEnumFORMATETC** ppEnum) override
    {
        if (dir != DATADIR_GET) return E_NOTIMPL;
        FORMATETC fe { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        return ::SHCreateStdEnumFmtEtc (1, &fe, ppEnum);
    }
    HRESULT STDMETHODCALLTYPE DAdvise   (FORMATETC*, DWORD, IAdviseSink*, DWORD*) override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE DUnadvise (DWORD)                                   override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE EnumDAdvise (IEnumSTATDATA**)                       override { return OLE_E_ADVISENOTSUPPORTED; }

private:
    HGLOBAL hDrop_ = nullptr;
    LONG    ref_   = 1;
};

class SimpleDropSource final : public IDropSource
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void** ppv) override
    {
        if (riid == IID_IUnknown || riid == IID_IDropSource)
            { *ppv = static_cast<IDropSource*> (this); AddRef(); return S_OK; }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef()  override { return (ULONG)::InterlockedIncrement (&ref_); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        LONG r = ::InterlockedDecrement (&ref_);
        if (r == 0) delete this;
        return (ULONG)r;
    }
    HRESULT STDMETHODCALLTYPE QueryContinueDrag (BOOL esc, DWORD keys) override
    {
        if (esc)                   return DRAGDROP_S_CANCEL;
        if (!(keys & MK_LBUTTON)) return DRAGDROP_S_DROP;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GiveFeedback (DWORD) override { return DRAGDROP_S_USEDEFAULTCURSORS; }

private:
    LONG ref_ = 1;
};

} // anonymous namespace
#endif  // JUCE_WINDOWS

// ── MIDI drag export ──────────────────────────────────────────────────────────

void SOLAR303Editor::exportAndDragMidi()
{
    const auto& seq = processor.getSequencer();

    constexpr int PPQ          = 480;
    constexpr int ticksPerStep = PPQ / 4;    // 120 ticks per 16th note

    const int len = std::max(1, std::min(16, seq.numSteps.load()));

    // ══════════════════════════════════════════════════════════════════════════
    //  RAW BINARY MIDI — no JUCE MidiFile class, no SysEx, no extra metadata
    //  Format 0, single track, clean as possible for Cubase import
    // ══════════════════════════════════════════════════════════════════════════

    // Helper: write variable-length quantity (VLQ) to a byte vector
    auto writeVLQ = [](std::vector<uint8_t>& buf, uint32_t val)
    {
        uint8_t bytes[4];
        int count = 0;
        bytes[count++] = val & 0x7F;
        while (val >>= 7)
        {
            bytes[count++] = (val & 0x7F) | 0x80;
        }
        for (int i = count - 1; i >= 0; --i)
            buf.push_back(bytes[i]);
    };

    // ── Build track data ────────────────────────────────────────────────────
    std::vector<uint8_t> trackData;

    // Meta events at delta=0 — required by Cubase to avoid phantom SysEx tracks
    // Tempo: 120 BPM = 500000 µs per beat (FF 51 03 07 A1 20)
    const uint8_t tempoEvt[] = { 0x00, 0xFF, 0x51, 0x03, 0x07, 0xA1, 0x20 };
    trackData.insert(trackData.end(), tempoEvt, tempoEvt + 7);

    // Time signature: 4/4 (FF 58 04 04 02 18 08)
    const uint8_t timeSigEvt[] = { 0x00, 0xFF, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08 };
    trackData.insert(trackData.end(), timeSigEvt, timeSigEvt + 8);

    // Collect all note events as (absoluteTick, type, note, velocity)
    struct MidiEvt { uint32_t tick; uint8_t status; uint8_t d1; uint8_t d2; };
    std::vector<MidiEvt> events;

    for (int i = 0; i < len; ++i)
    {
        const auto& s = seq.steps[i];
        if (!s.active) continue;

        const uint32_t startTick = (uint32_t)(i * ticksPerStep);
        const uint8_t vel = s.accent ? (uint8_t)127 : (uint8_t)70;

        // Slide: note extends slightly past next step boundary (overlap = legato)
        // No slide: note ends 2 ticks before next step (small gap = staccato)
        const uint32_t durTicks = s.slide
            ? (uint32_t)(ticksPerStep + 2)
            : (uint32_t)(ticksPerStep - 2);

        events.push_back({ startTick, 0x90, (uint8_t)s.note, vel });           // Note On ch1
        events.push_back({ startTick + durTicks, 0x80, (uint8_t)s.note, 64 }); // Note Off ch1
    }

    // Sort by tick (stable sort preserves note-on before note-off at same tick)
    std::stable_sort(events.begin(), events.end(),
        [](const MidiEvt& a, const MidiEvt& b) { return a.tick < b.tick; });

    // Write events as delta-time + status + data1 + data2
    uint32_t prevTick = 0;
    for (const auto& e : events)
    {
        uint32_t delta = e.tick - prevTick;
        prevTick = e.tick;
        writeVLQ(trackData, delta);
        trackData.push_back(e.status);
        trackData.push_back(e.d1);
        trackData.push_back(e.d2);
    }

    // End-of-track meta event: delta=0, FF 2F 00
    uint32_t endTick = (uint32_t)(len * ticksPerStep);
    uint32_t endDelta = endTick - prevTick;
    writeVLQ(trackData, endDelta);
    trackData.push_back(0xFF);
    trackData.push_back(0x2F);
    trackData.push_back(0x00);

    // ── Assemble complete MIDI file ─────────────────────────────────────────
    std::vector<uint8_t> midiBytes;

    // MThd header: 14 bytes
    // "MThd" + length(6) + format(0) + nTracks(1) + division(PPQ)
    const uint8_t header[] = {
        'M','T','h','d',
        0x00, 0x00, 0x00, 0x06,         // header length = 6
        0x00, 0x00,                       // format 0
        0x00, 0x01,                       // 1 track
        (uint8_t)((PPQ >> 8) & 0xFF),     // ticks per quarter note (high byte)
        (uint8_t)(PPQ & 0xFF)             // ticks per quarter note (low byte)
    };
    midiBytes.insert(midiBytes.end(), header, header + 14);

    // MTrk chunk: "MTrk" + 4-byte length + track data
    uint32_t trkLen = (uint32_t)trackData.size();
    const uint8_t trkHeader[] = {
        'M','T','r','k',
        (uint8_t)((trkLen >> 24) & 0xFF),
        (uint8_t)((trkLen >> 16) & 0xFF),
        (uint8_t)((trkLen >> 8)  & 0xFF),
        (uint8_t)(trkLen & 0xFF)
    };
    midiBytes.insert(midiBytes.end(), trkHeader, trkHeader + 8);
    midiBytes.insert(midiBytes.end(), trackData.begin(), trackData.end());

    // ── Write to temp file ──────────────────────────────────────────────────
    const auto midiFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                              .getChildFile ("SOLAR303_Pattern.mid");
    midiFile.deleteFile();
    {
        juce::FileOutputStream fos (midiFile);
        if (fos.openedOk())
        {
            fos.write (midiBytes.data(), midiBytes.size());
            fos.flush();
        }
    }

    // ── Drag the file into the DAW ───────────────────────────────────────────
    // On Windows we use a native COM IDataObject that exposes ONLY CF_HDROP.
    // This is exactly what Windows Explorer produces — no extra MIME formats
    // that could cause Cubase to create phantom SysEx Data tracks.
#if JUCE_WINDOWS
    {
        auto* dataObj = new HDropDataObject (midiFile.getFullPathName());
        auto* dropSrc = new SimpleDropSource();
        DWORD dropEffect = 0;
        ::DoDragDrop (dataObj, dropSrc,
                      DROPEFFECT_COPY | DROPEFFECT_MOVE,
                      &dropEffect);
        dataObj->Release();
        dropSrc->Release();
    }
#else
    juce::DragAndDropContainer::performExternalDragDropOfFiles (
        { midiFile.getFullPathName() }, false, nullptr, nullptr);
#endif
}

// ── RND History helpers ───────────────────────────────────────────────────────

SOLAR303Editor::PatternSnapshot SOLAR303Editor::captureSnapshot()
{
    PatternSnapshot snap;
    const auto& seq   = processor.getSequencer();
    const auto& apvts = processor.getAPVTS();

    for (int i = 0; i < 16; ++i)
    {
        snap.steps[i].note   = seq.steps[i].note;
        snap.steps[i].accent = seq.steps[i].accent;
        snap.steps[i].slide  = seq.steps[i].slide;
        snap.steps[i].active = seq.steps[i].active;
    }

    auto getN = [&](const juce::String& id) -> float
    {
        if (auto* p = apvts.getParameter(id)) return p->getValue();
        return 0.f;
    };
    snap.cutoff    = getN("cutoff");
    snap.resonance = getN("resonance");
    snap.envmod    = getN("envmod");
    snap.decay     = getN("decay");
    snap.accentVal = getN("accent");
    snap.slideVal  = getN("slide");

    return snap;
}

void SOLAR303Editor::applySnapshot(const PatternSnapshot& snap)
{
    auto& seq   = processor.getSequencer();
    auto& apvts = processor.getAPVTS();

    for (int i = 0; i < 16; ++i)
    {
        seq.steps[i].note   = snap.steps[i].note;
        seq.steps[i].accent = snap.steps[i].accent;
        seq.steps[i].slide  = snap.steps[i].slide;
        seq.steps[i].active = snap.steps[i].active;

        updateNoteButton(i);
        accentButtons[i].setToggleState(snap.steps[i].accent, juce::dontSendNotification);
        slideButtons [i].setToggleState(snap.steps[i].slide,  juce::dontSendNotification);
        activeButtons[i].setToggleState(snap.steps[i].active, juce::dontSendNotification);
    }

    auto setN = [&](const juce::String& id, float v)
    {
        if (auto* p = apvts.getParameter(id)) p->setValueNotifyingHost(v);
    };
    setN("cutoff",    snap.cutoff);
    setN("resonance", snap.resonance);
    setN("envmod",    snap.envmod);
    setN("decay",     snap.decay);
    setN("accent",    snap.accentVal);
    setN("slide",     snap.slideVal);
}

void SOLAR303Editor::pushRndSnapshot()
{
    // Truncate forward history when a new branch starts
    if (rndHistIdx < (int)rndHistory.size() - 1)
        rndHistory.erase(rndHistory.begin() + rndHistIdx + 1, rndHistory.end());

    // Ensure the very first press also saves the initial state
    if (rndHistory.empty())
    {
        rndHistory.push_back(captureSnapshot());
        rndHistIdx = 0;
    }
    // Post-random state is saved by finaliseRndSnapshot()
}

void SOLAR303Editor::finaliseRndSnapshot()
{
    rndHistory.push_back(captureSnapshot());
    rndHistIdx = (int)rndHistory.size() - 1;

    // Cap history at 64 entries
    while ((int)rndHistory.size() > 64)
    {
        rndHistory.erase(rndHistory.begin());
        if (rndHistIdx > 0) --rndHistIdx;
    }

    updateRndNavButtons();
}

void SOLAR303Editor::updateRndNavButtons()
{
    rndBackButton.setEnabled(rndHistIdx > 0);
    rndFwdButton .setEnabled(rndHistIdx < (int)rndHistory.size() - 1);
}

// ── Note range helper — "C1", "F#3", etc. (Cubase: C3 = MIDI 60) ────────────
juce::String SOLAR303Editor::midiNoteName(int midi)
{
    static const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return juce::String(names[midi % 12]) + juce::String(midi / 12 - 2);
}

// ── Note range picker popup ───────────────────────────────────────────────────
void SOLAR303Editor::showNoteRangePicker(bool isLow)
{
    static const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

    juce::PopupMenu menu;
    for (int oct = 0; oct <= 5; ++oct)
    {
        juce::PopupMenu sub;
        for (int n = 0; n < 12; ++n)
        {
            const int midi = (oct + 2) * 12 + n;   // C0=24 … (Cubase C3=60)
            if (midi > 84) break;
            sub.addItem(midi + 1, juce::String(names[n]) + juce::String(oct));
        }
        menu.addSubMenu("Oct " + juce::String(oct), sub);
    }

    auto& targetBtn = isLow ? rndLowNoteButton : rndHighNoteButton;

    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(&targetBtn),
        [this, isLow](int result)
        {
            if (result <= 0) return;
            const int chosen = result - 1;
            if (isLow)
            {
                rndMinNote = juce::jmin(chosen, rndMaxNote);   // can't exceed HIGH
                rndLowNoteButton.setButtonText(midiNoteName(rndMinNote));
            }
            else
            {
                rndMaxNote = juce::jmax(chosen, rndMinNote);   // can't go below LOW
                rndHighNoteButton.setButtonText(midiNoteName(rndMaxNote));
            }
        });
}
