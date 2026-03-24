#pragma once
#include <JuceHeader.h>

//==============================================================================
// ActivationScreen — shown when the plugin is not yet activated
//==============================================================================
class ActivationScreen : public juce::Component,
                         private juce::Thread
{
public:
    std::function<void()> onActivated;

    ActivationScreen();
    ~ActivationScreen() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    juce::Label      titleLabel, subtitleLabel, instructionLabel, statusLabel;
    juce::TextEditor keyInput;
    juce::TextButton activateBtn { "ACTIVATE" };

    juce::String pendingKey;

    void attemptActivation();
    void run() override;   // juce::Thread — network call here

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActivationScreen)
};
