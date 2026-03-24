#include "ActivationScreen.h"
#include "LicenseManager.h"

static const juce::Colour kOrange  { 0xffff9900 };
static const juce::Colour kCyan    { 0xff00e5ff };
static const juce::Colour kBg      { 0xff020408 };
static const juce::Colour kRed     { 0xffdd4444 };
static const juce::Colour kGreen   { 0xff00cc44 };
static const juce::Colour kDimText { 0xff888880 };

ActivationScreen::ActivationScreen()
    : juce::Thread ("LicenseVerifyThread")
{
    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel.setText ("SOLAR 303", juce::dontSendNotification);
    titleLabel.setFont (juce::Font ("Arial", 36.f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, kOrange);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("THE ACID MACHINE", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font ("Arial", 11.f, juce::Font::plain));
    subtitleLabel.setJustificationType (juce::Justification::centred);
    subtitleLabel.setColour (juce::Label::textColourId, kCyan);
    addAndMakeVisible (subtitleLabel);

    // ── Instruction ───────────────────────────────────────────────────────────
    instructionLabel.setText ("Enter your Gumroad license key to activate:",
                              juce::dontSendNotification);
    instructionLabel.setFont (juce::Font ("Arial", 12.f, juce::Font::plain));
    instructionLabel.setJustificationType (juce::Justification::centred);
    instructionLabel.setColour (juce::Label::textColourId, kDimText);
    addAndMakeVisible (instructionLabel);

    // ── Key input ─────────────────────────────────────────────────────────────
    keyInput.setMultiLine (false);
    keyInput.setReturnKeyStartsNewLine (false);
    keyInput.setTextToShowWhenEmpty ("XXXX-XXXX-XXXX-XXXX-XXXX", kDimText);
    keyInput.setJustification (juce::Justification::centred);
    keyInput.setFont (juce::Font ("Courier New", 14.f, juce::Font::plain));
    keyInput.setColour (juce::TextEditor::backgroundColourId,     juce::Colour (0xff080c10));
    keyInput.setColour (juce::TextEditor::textColourId,           juce::Colour (0xffe0ddd8));
    keyInput.setColour (juce::TextEditor::outlineColourId,        kOrange.withAlpha (0.35f));
    keyInput.setColour (juce::TextEditor::focusedOutlineColourId, kOrange);
    keyInput.onReturnKey = [this] { attemptActivation(); };
    addAndMakeVisible (keyInput);

    // ── Activate button ───────────────────────────────────────────────────────
    activateBtn.setColour (juce::TextButton::buttonColourId,   kOrange);
    activateBtn.setColour (juce::TextButton::buttonOnColourId, kOrange.darker (0.2f));
    activateBtn.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xff000000));
    activateBtn.setColour (juce::TextButton::textColourOnId,   juce::Colour (0xff000000));
    activateBtn.onClick = [this] { attemptActivation(); };
    addAndMakeVisible (activateBtn);

    // ── Status label ──────────────────────────────────────────────────────────
    statusLabel.setFont (juce::Font ("Arial", 11.f, juce::Font::plain));
    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setColour (juce::Label::textColourId, kRed);
    addAndMakeVisible (statusLabel);
}

ActivationScreen::~ActivationScreen()
{
    stopThread (5000);
}

//==============================================================================
void ActivationScreen::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (kBg);

    // Subtle orange radial glow at centre
    auto cx = (float) getWidth()  * 0.5f;
    auto cy = (float) getHeight() * 0.5f;
    juce::ColourGradient glow (kOrange.withAlpha (0.08f), cx, cy,
                                juce::Colours::transparentBlack, cx + 320.f, cy, true);
    g.setGradientFill (glow);
    g.fillAll();

    // Outer border
    g.setColour (kOrange.withAlpha (0.25f));
    g.drawRect (getLocalBounds().reduced (16), 1);

    // Inner corner brackets
    const int bs = 16, bt = 2, pad = 22;
    g.setColour (kOrange.withAlpha (0.7f));
    // TL
    g.fillRect (pad,      pad,      bs, bt);
    g.fillRect (pad,      pad,      bt, bs);
    // TR
    g.fillRect (getWidth() - pad - bs, pad, bs, bt);
    g.fillRect (getWidth() - pad - bt, pad, bt, bs);
    // BL
    g.fillRect (pad, getHeight() - pad - bt, bs, bt);
    g.fillRect (pad, getHeight() - pad - bs, bt, bs);
    // BR
    g.fillRect (getWidth() - pad - bs, getHeight() - pad - bt, bs, bt);
    g.fillRect (getWidth() - pad - bt, getHeight() - pad - bs, bt, bs);

    // Horizontal divider below subtitle
    g.setColour (kOrange.withAlpha (0.15f));
    g.drawHorizontalLine (getHeight() / 2 - 60, (float) pad + 20.f, (float) getWidth() - pad - 20.f);
}

void ActivationScreen::resized()
{
    auto area = getLocalBounds().reduced (60, 0);
    auto centreY = getHeight() / 2;

    titleLabel       .setBounds (area.withY (centreY - 160).withHeight (44));
    subtitleLabel    .setBounds (area.withY (centreY - 118).withHeight (20));
    instructionLabel .setBounds (area.withY (centreY -  70).withHeight (20));

    auto inputBounds = area.withY (centreY - 44).withHeight (34);
    keyInput.setBounds (inputBounds);

    auto btnBounds = juce::Rectangle<int> (0, centreY + 8, 180, 36)
                         .withX ((getWidth() - 180) / 2);
    activateBtn.setBounds (btnBounds);

    statusLabel.setBounds (area.withY (centreY + 56).withHeight (24));
}

//==============================================================================
void ActivationScreen::attemptActivation()
{
    pendingKey = keyInput.getText().trim();

    if (pendingKey.isEmpty())
    {
        statusLabel.setColour (juce::Label::textColourId, kRed);
        statusLabel.setText ("Please enter your license key.", juce::dontSendNotification);
        return;
    }

    activateBtn.setEnabled (false);
    statusLabel.setColour (juce::Label::textColourId, kCyan);
    statusLabel.setText ("Verifying\xe2\x80\xa6", juce::dontSendNotification);  // "Verifying…"

    if (isThreadRunning()) stopThread (2000);
    startThread();
}

// ── Background thread: network call ──────────────────────────────────────────
void ActivationScreen::run()
{
    juce::String error;
    const bool ok = LicenseManager::verifyWithGumroad (pendingKey, error);

    juce::MessageManager::callAsync ([this, ok, error]()
    {
        if (ok)
        {
            statusLabel.setColour (juce::Label::textColourId, kGreen);
            statusLabel.setText ("Activated! Thank you.", juce::dontSendNotification);

            // Small delay so user sees the success message
            juce::Timer::callAfterDelay (800, [this]()
            {
                if (onActivated) onActivated();
            });
        }
        else
        {
            statusLabel.setColour (juce::Label::textColourId, kRed);
            statusLabel.setText (error, juce::dontSendNotification);
            activateBtn.setEnabled (true);
        }
    });
}
