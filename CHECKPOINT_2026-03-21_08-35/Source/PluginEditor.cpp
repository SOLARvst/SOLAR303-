#include "PluginEditor.h"
#include <BinaryData.h>

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
    else if (b.getProperties().contains("dist"))
        col = on ? LedWhite : TextOnBtn.withAlpha(0.8f);
    else if (b.getProperties().contains("delaynote"))
        col = on ? LedGreen : TextOnBtn.withAlpha(0.75f);
    else
        col = b.findColour(juce::TextButton::textColourOffId);

    g.setColour(col);
    const float fs = (float)b.getProperties().getWithDefault("fontSize", 9.5f);
    g.setFont(juce::Font("Arial", fs, juce::Font::bold));
    g.drawFittedText(b.getButtonText(), b.getLocalBounds(),
                     juce::Justification::centred, 1);
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
        const char* ids[] = {
            "cutoff","resonance","envmod","decay","accent","slide","volume",
            nullptr
        };
        const float v = resetKnobsTo50 ? 0.5f : 0.0f;
        for (int i = 0; ids[i]; ++i)
            if (auto* p = processor.getAPVTS().getParameter(ids[i]))
            { p->beginChangeGesture(); p->setValueNotifyingHost(v); p->endChangeGesture(); }
        resetKnobsTo50 = !resetKnobsTo50;
        resetKnobsButton.setButtonText(resetKnobsTo50 ? "RST 50%" : "RST 0%");
    };
    addAndMakeVisible(resetKnobsButton);

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
            "Save Preset", presetsDir, "*.solar;*.zb303;*.vstpreset;*.tb303");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File{}) return;

                if (f.getFileExtension().isEmpty())
                    f = f.withFileExtension(".solar");

                juce::MemoryBlock block;
                processor.getStateInformation(block);
                f.replaceWithData(block.getData(), block.getSize());
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

    // ── HI-PASS FX section ────────────────────────────────────────────────────
    static const float       hpfFreqs[]  = { 30.f, 50.f, 100.f, 125.f, 150.f };
    static const char* const hpfLabels[] = { "30 Hz", "50 Hz", "100 Hz", "125 Hz", "150 Hz" };

    // ON/OFF toggle — green when ON
    hpfOnButton.setColour(juce::TextButton::buttonColourId, AcidColors::BtnOff);
    setHpfBtnActive(hpfOnButton, false);   // starts OFF → dim
    hpfOnButton.onClick = [this]()
    {
        hpfActive = !hpfActive;
        if (hpfActive)
        {
            processor.hpfFreq.store(hpfFreqs[hpfFreqState]);
            hpfOnButton.setButtonText("HPF ON");
        }
        else
        {
            processor.hpfFreq.store(0.f);
            hpfOnButton.setButtonText("HPF OFF");
        }
        setHpfBtnActive(hpfOnButton, hpfActive);
    };
    addAndMakeVisible(hpfOnButton);

    // Frequency radio buttons — one lights up green at a time
    for (int i = 0; i < 5; ++i)
    {
        hpfFreqButtons[i].setButtonText(hpfLabels[i]);
        hpfFreqButtons[i].setColour(juce::TextButton::buttonColourId, AcidColors::BtnOff);
        setHpfBtnActive(hpfFreqButtons[i], i == hpfFreqState);   // index 0 active by default

        hpfFreqButtons[i].onClick = [this, i]()
        {
            hpfFreqState = i;
            if (hpfActive)
                processor.hpfFreq.store(hpfFreqs[i]);
            for (int j = 0; j < 5; ++j)
                setHpfBtnActive(hpfFreqButtons[j], j == i);
        };
        addAndMakeVisible(hpfFreqButtons[i]);
    }

    // Slope buttons — 24 dB active by default, green when selected
    hpf24Button.setColour(juce::TextButton::buttonColourId, AcidColors::BtnOff);
    setHpfBtnActive(hpf24Button, true);    // 24 dB selected by default
    hpf24Button.onClick = [this]()
    {
        hpfSlopeIdx = 0;
        processor.hpfSlope.store(0);
        setHpfBtnActive(hpf24Button, true);
        setHpfBtnActive(hpf48Button, false);
    };
    addAndMakeVisible(hpf24Button);

    hpf48Button.setColour(juce::TextButton::buttonColourId, AcidColors::BtnOff);
    setHpfBtnActive(hpf48Button, false);
    hpf48Button.onClick = [this]()
    {
        hpfSlopeIdx = 1;
        processor.hpfSlope.store(1);
        setHpfBtnActive(hpf48Button, true);
        setHpfBtnActive(hpf24Button, false);
    };
    addAndMakeVisible(hpf48Button);

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

    lWave.setText("WAVE", juce::dontSendNotification);
    lWave.setJustificationType(juce::Justification::centred);
    lWave.setColour(juce::Label::textColourId, LedWhite);
    lWave.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(lWave);

    // ── Distortion controls ───────────────────────────────────────────────────
    distOnButton.setClickingTogglesState(true);
    distOnButton.getProperties().set("dist", true);   // custom style — not play/stop
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
    lDistType.setColour(juce::Label::textColourId, TextOnBtn);
    lDistType.setFont(juce::Font("Arial", 8.f, juce::Font::bold));
    addAndMakeVisible(lDistType);

    setupKnob(sDistAmount, lDistAmount, "AMOUNT");
    setupKnob(sDistVol,    lDistVol,    "DIST VOL");

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
        delayOnButton.setButtonText(
            delayOnButton.getToggleState() ? "DLY ON" : "DLY OFF");
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

    startTimerHz(20);
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

    // ── Branding ──────────────────────────────────────────────────────────────
    // "z b 303" — header top-left, "z b" in EasyRide, "303" in Arial Bold, white 50%
    g.setColour(juce::Colours::white.withAlpha(0.50f));
    g.setFont(easyRide(16.f));
    g.drawText("SOLAR  303", 10, 5, 120, 26, juce::Justification::centredLeft);

    // "made by" — right side, aAcakadut 14pt, white 50%
    g.setFont(acakadut(14.f));
    g.setColour(juce::Colours::white.withAlpha(0.50f));
    g.drawText("made by", 708, 196, 88, 18, juce::Justification::centredLeft);

    // "SOLAR" — right side large, EasyRide 30pt, white 50%
    g.setFont(easyRide(30.f));
    g.setColour(juce::Colours::white.withAlpha(0.50f));
    g.drawText("SOLAR", 756, 183, 135, 40, juce::Justification::centredLeft);

    // "vst" — below SOLAR right, aAcakadut 18pt, white 46%
    g.setFont(acakadut(18.f));
    g.setColour(juce::Colours::white.withAlpha(0.46f));
    g.drawText("vst", 845, 200, 44, 22, juce::Justification::centredLeft);


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

    // ── Distortion box ────────────────────────────────────────────────────────
    g.setColour(BtnOff);
    g.fillRoundedRectangle(692.f, 252.f, 204.f, 118.f, 4.f);
    g.setColour(BtnRim);
    g.drawRoundedRectangle(692.f, 252.f, 204.f, 118.f, 4.f, 1.5f);
    // Red label stripe at top of distortion box — gradient top→bottom
    {
        juce::Rectangle<float> labelStripe(692.f, 252.f, 204.f, 16.f);
        g.setGradientFill(juce::ColourGradient(
            RolandRed.brighter(0.35f), 692.f, 252.f,
            RolandRed.darker  (0.45f), 692.f, 268.f, false));
        g.fillRoundedRectangle(labelStripe, 4.f);
        g.fillRect(692.f, 260.f, 204.f, 8.f);
    }
    g.setColour(TextOnBtn);
    g.setFont(juce::Font("Arial", 8.5f, juce::Font::bold));
    g.drawText("DISTORTION", 692, 252, 204, 16, juce::Justification::centred);

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

    drawFxBox( 10, 385, 230,  78, "REVERB");
    drawFxBox(248, 385, 230, 118, "DELAY");    // taller — room for note-div presets
    drawFxBox(486, 385, 250, 100, "HI-PASS");  // taller — 2 rows of buttons

    // Row labels for delay note presets
    g.setColour(TextOnBtn.withAlpha(0.55f));
    g.setFont(juce::Font("Arial", 7.5f, juce::Font::bold));
    g.drawText("SRT", 252, 385 + 68, 30, 12, juce::Justification::centredRight);
    g.drawText("T",   252, 385 + 82, 30, 12, juce::Justification::centredRight);
    g.drawText("D",   252, 385 + 96, 30, 12, juce::Justification::centredRight);
}

// ── Resized ───────────────────────────────────────────────────────────────────

void SOLAR303Editor::resized()
{
    constexpr int stepW  = 50;
    constexpr int startX = 40;

    // ── Header row ────────────────────────────────────────────────────────────
    // Button group — centred in 900 px, same y as DRAG MIDI button (y=191)
    playButton        .setBounds(265, 191,  60, 22);
    randomButton      .setBounds(388, 191,  44, 22);   // REND A
    randomKnobButton  .setBounds(436, 191,  44, 22);   // REND K
    randomStepsButton .setBounds(484, 191,  44, 22);   // REND S
    rndFwdButton      .setBounds(532, 191,  22, 22);   // ►
    rndBackButton     .setBounds(558, 191,  22, 22);   // ◄
    resetKnobsButton  .setBounds(555, 335, 90, 22);   // RST — below SAW/SQ waveform combo

    // VST3: EXPORT + IMPORT in header; DRAG MIDI in the empty strip below step grid
    dragMidiButton      .setBounds(40, 191, 120, 22);   // aligned with first step key (x=40)
    exportPresetButton  .setBounds(393,  6,  55, 24);
    importPresetButton  .setBounds(452,  6,  55, 24);
    // Standalone: SAVE + LOAD + EXPLORER
    savePresetButton    .setBounds(375,  6,  55, 24);
    loadPresetButton    .setBounds(434,  6,  55, 24);
    showExplorerButton  .setBounds(493,  6,  75, 24);

    bpmLabel .setBounds(118, 10,  30, 16);
    bpmSlider.setBounds(146,  4, 104, 30);
    // STEPS — parallel (label + slider side by side), left edge aligned with PLAY button (x=265)
    lengthLabel .setBounds(231, 160, 30, 14);
    lengthSlider.setBounds(265, 156, 82, 22);
    skinButton.setBounds(736, 6,  60, 24);

    // (hpfButton moved to FX section — see below)

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

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int col)
    {
        s.setBounds(col * kW + 8, kY,     kW - 4, kH);
        l.setBounds(col * kW + 8, kY + kH + 2, kW - 4, lH);
    };

    placeKnob(sCutoff,    lCutoff,    0);
    placeKnob(sResonance, lResonance, 1);
    placeKnob(sEnvMod,    lEnvMod,    2);
    placeKnob(sDecay,     lDecay,     3);
    placeKnob(sAccent,    lAccent,    4);
    placeKnob(sSlide,     lSlide,     5);

    // ── ML UP/DOWN buttons — below each knob label ──────────────────────────
    constexpr int mlBtnW = 28;   // each button width
    constexpr int mlBtnH = 15;   // button height
    constexpr int mlGap  = 2;    // gap between UP and DOWN
    constexpr int mlY    = kY + kH + lH + 4;
    for (int i = 0; i < 6; ++i)
    {
        const int cx = i * kW + 8 + (kW - 4) / 2;  // centre of knob column
        mlDownButtons[i].setBounds(cx - mlBtnW - mlGap / 2, mlY, mlBtnW, mlBtnH);
        mlUpButtons  [i].setBounds(cx + mlGap / 2,           mlY, mlBtnW, mlBtnH);
    }

    // TUNE: centred in the gap between SLIDE end (x=436) and WAVE area (x=550)
    // centre = (436+550)/2 = 493, knob x = 493 - 34 = 459
    sTuning.setBounds(459, kY,      kW - 4, kH);
    lTuning.setBounds(459, kY + kH + 2, kW - 4, lH);

    // Waveform — centred in the gap between TUNE area and DIST box (x=692)
    // centre = (508+692)/2 = 600; combo 100px → x=550; label 70px → x=565
    lWave   .setBounds(565, kY + 10, 70, 13);
    waveCombo.setBounds(550, kY + 26, 100, 24);

    // ── Distortion section ────────────────────────────────────────────────────
    // Box: x=692, y=252, w=204, h=118
    // Left column (x=696..776): DIST ON button + TYPE label + TYPE combo
    // Right columns: AMOUNT knob + VOL knob
    constexpr int dX  = 692;
    constexpr int dkW = 56;   // distortion knob width
    constexpr int lkH = 62;   // distortion knob height (shorter than main knobs)

    distOnButton  .setBounds(dX + 4,  kY + 18, 80, 22);
    lDistType     .setBounds(dX + 4,  kY + 44, 80, 11);
    distTypeCombo .setBounds(dX + 4,  kY + 56, 80, 20);

    sDistAmount   .setBounds(dX + 88, kY + 10, dkW, lkH);
    lDistAmount   .setBounds(dX + 88, kY + 10 + lkH - 2, dkW, lH);

    sDistVol      .setBounds(dX + 148, kY + 10, 52, lkH);
    lDistVol      .setBounds(dX + 148, kY + 10 + lkH - 2, 52, lH);

    // ── FX section (y=385, two boxes: REVERB x=10 and DELAY x=248) ───────────
    constexpr int fxY  = 385;    // top of FX section (below separator at 382)
    constexpr int fkH  = 52;     // FX knob height
    constexpr int fkW  = 54;     // FX knob width
    constexpr int flH  = 12;     // FX label height

    // REVERB box (x=10, w=230)
    revOnButton.setBounds(14,   fxY + 18, 58, 20);
    lRevOn     .setBounds(14,   fxY + 40, 58, 11);
    sRevSize   .setBounds(78,   fxY +  6, fkW, fkH);
    lRevSize   .setBounds(78,   fxY +  6 + fkH - 2, fkW, flH);
    sRevDamp   .setBounds(136,  fxY +  6, fkW, fkH);
    lRevDamp   .setBounds(136,  fxY +  6 + fkH - 2, fkW, flH);
    sRevMix    .setBounds(190,  fxY +  6, 46, fkH);
    lRevMix    .setBounds(190,  fxY +  6 + fkH - 2, 46, flH);

    // DELAY box (x=248, w=230)
    delayOnButton.setBounds(252, fxY + 18, 58, 20);
    lDelayOn     .setBounds(252, fxY + 40, 58, 11);
    sDelayTime   .setBounds(316, fxY +  6, fkW, fkH);
    lDelayTime   .setBounds(316, fxY +  6 + fkH - 2, fkW, flH);
    sDelayFeed   .setBounds(374, fxY +  6, fkW, fkH);
    lDelayFeed   .setBounds(374, fxY +  6 + fkH - 2, fkW, flH);
    sDelayMix    .setBounds(430, fxY +  6, 44, fkH);
    lDelayMix    .setBounds(430, fxY +  6 + fkH - 2, 44, flH);

    // ── Delay note-division preset buttons (3 rows × 4 cols inside DELAY box) ──
    constexpr int dnX0 = 284;   // starts after the SRT/T/D labels
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

    // ── HI-PASS FX box (x=486, w=250, h=100) — two rows ─────────────────────
    // Row 1 (y+19): HPF ON/OFF | 24 dB | 48 dB
    {
        constexpr int r1y = fxY + 19;
        constexpr int r1h = 20;
        hpfOnButton .setBounds(490, r1y, 72, r1h);
        hpf24Button .setBounds(566, r1y, 76, r1h);
        hpf48Button .setBounds(646, r1y, 76, r1h);
    }
    // Row 2 (y+45): 30Hz | 50Hz | 100Hz | 125Hz | 150Hz — equal width radio
    {
        constexpr int r2y  = fxY + 45;
        constexpr int r2h  = 20;
        constexpr int btnW = 44;
        constexpr int gap  = 3;
        for (int i = 0; i < 5; ++i)
            hpfFreqButtons[i].setBounds(490 + i * (btnW + gap), r2y, btnW, r2h);
    }

    // ── Master Volume (right side — after HI-PASS ends at x=736, window=900) ──
    // centre of 736..900 = 818; knob 80px → x=778
    sMasterVol.setBounds(778, fxY + 2, 80, 70);
    lMasterVol.setBounds(778, fxY + 2 + 70 + 2, 80, flH);   // 2px below knob bottom, like other knobs

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

        // Note: pick from pentatonic pool, root randomised per press (C–B)
        const int rootOffset = rng.nextInt(12);   // random key each press
        step.note    = 24 + rootOffset + penta[rng.nextInt(numPenta)];
        step.note    = juce::jlimit(24, 84, step.note);

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

        const int rootOffset = rng.nextInt(12);
        step.note   = 24 + rootOffset + penta[rng.nextInt(numPenta)];
        step.note   = juce::jlimit(24, 84, step.note);

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

// ── MIDI drag export ──────────────────────────────────────────────────────────

void SOLAR303Editor::exportAndDragMidi()
{
    const auto& seq = processor.getSequencer();
    const float bpm = seq.bpm.load();

    constexpr int PPQ          = 96;
    constexpr int ticksPerStep = PPQ / 4;   // 24 ticks per 16th note
    const int microsPerBeat    = juce::roundToInt(60000000.0 / bpm);

    // ── Collect note-on / note-off events ─────────────────────────────────────
    struct Ev { int tick; juce::MidiMessage msg; };
    std::vector<Ev> events;

    const int len = std::max(1, std::min(16, seq.numSteps.load()));
    for (int i = 0; i < len; ++i)
    {
        const auto& s = seq.steps[i];
        if (!s.active) continue;

        const int start = i * ticksPerStep;
        const juce::uint8 vel = s.accent ? (juce::uint8)127 : (juce::uint8)70;
        const int dur   = s.slide ? (ticksPerStep + 2) : (ticksPerStep - 2);

        events.push_back({ start,       juce::MidiMessage::noteOn (1, s.note, vel)              });
        events.push_back({ start + dur, juce::MidiMessage::noteOff(1, s.note, (juce::uint8)64) });
    }

    std::sort(events.begin(), events.end(),
              [](const Ev& a, const Ev& b) { return a.tick < b.tick; });

    // ── Write Format 0 MIDI manually (1 track = 1 clip in every DAW) ─────────
    // Using raw binary avoids JUCE MidiFile writing Format 1 by default.

    // Helper: variable-length quantity encoding
    auto writeVarLen = [](juce::OutputStream& out, int v)
    {
        if (v < 0x80) {
            out.writeByte((juce::uint8)v);
        } else if (v < 0x4000) {
            out.writeByte((juce::uint8)((v >> 7) | 0x80));
            out.writeByte((juce::uint8)( v        & 0x7F));
        } else {
            out.writeByte((juce::uint8)((v >> 14) | 0x80));
            out.writeByte((juce::uint8)(((v >>  7) & 0x7F) | 0x80));
            out.writeByte((juce::uint8)( v          & 0x7F));
        }
    };

    // ── Build track data ──────────────────────────────────────────────────────
    juce::MemoryOutputStream trackData;

    // Tempo meta event at tick 0
    writeVarLen(trackData, 0);
    trackData.writeByte(0xFF); trackData.writeByte(0x51); trackData.writeByte(0x03);
    trackData.writeByte((juce::uint8)((microsPerBeat >> 16) & 0xFF));
    trackData.writeByte((juce::uint8)((microsPerBeat >>  8) & 0xFF));
    trackData.writeByte((juce::uint8)( microsPerBeat        & 0xFF));

    // Note events
    int prevTick = 0;
    for (const auto& ev : events)
    {
        writeVarLen(trackData, ev.tick - prevTick);
        prevTick = ev.tick;
        trackData.write(ev.msg.getRawData(), (size_t)ev.msg.getRawDataSize());
    }

    // End-of-track meta event
    const int endTick = 16 * ticksPerStep;
    writeVarLen(trackData, endTick - prevTick);
    trackData.writeByte(0xFF); trackData.writeByte(0x2F); trackData.writeByte(0x00);

    // ── Assemble MIDI file (Format 0, 1 track) ────────────────────────────────
    juce::MemoryOutputStream midiOut;
    midiOut.write("MThd", 4);
    midiOut.writeIntBigEndian(6);
    midiOut.writeShortBigEndian(0);                          // Format 0 — single track
    midiOut.writeShortBigEndian(1);                          // 1 track
    midiOut.writeShortBigEndian((short)PPQ);                 // ticks per quarter note

    midiOut.write("MTrk", 4);
    midiOut.writeIntBigEndian((int)trackData.getDataSize());
    midiOut.write(trackData.getData(), trackData.getDataSize());

    // ── Write to temp file & drag ─────────────────────────────────────────────
    const auto tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                              .getChildFile("SOLAR303_Pattern.mid");
    tempFile.replaceWithData(midiOut.getData(), midiOut.getDataSize());

    juce::DragAndDropContainer::performExternalDragDropOfFiles(
        { tempFile.getFullPathName() }, false, &dragMidiButton, nullptr);
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
