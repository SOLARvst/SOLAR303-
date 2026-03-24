#pragma once
#include <JuceHeader.h>
#include "SOLAR303Engine.h"
#include "SOLAR303Sequencer.h"

class SOLAR303Processor : public juce::AudioProcessor,
                          private juce::Timer
{
public:
    SOLAR303Processor();
    ~SOLAR303Processor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SOLAR303"; }
    bool  acceptsMidi()  const override { return true; }
    bool  producesMidi() const override { return false; }
    bool  isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int  getNumPrograms()    override;
    int  getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void loadFactoryPreset (int index);

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    SOLAR303Sequencer& getSequencer() { return sequencer; }

    // ── Note preview (editor → audio thread, lock-free) ──────────────────────
    void previewNoteOn (int midiNote, float velocity = 0.5f, bool slide = false)
    {
        previewNoteSlide .store (slide ? 1 : 0);
        previewNoteOnNote.store (midiNote);
        previewNoteOnVel .store (velocity);
    }
    void previewNoteOff(int midiNote)
    {
        previewNoteOffNote.store(midiNote);
    }

    void timerCallback() override;

private:
    int currentProgram = 0;
    juce::Time lastPresetDirMTime;

    SOLAR303Engine    engine;
    SOLAR303Sequencer sequencer;
    bool prevSeqPlaying  { false };

    juce::Reverb             reverb;
    juce::Reverb::Parameters cachedRevParams { -1.f,-1.f,-1.f,1.f,0.85f,0.f }; // invalid → forces 1st update
    std::vector<float> delayBuf;
    int                delayWritePos { 0 };

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<bool>  bpmSyncedFromHost  { false }; // true after first processBlock BPM read

public:
    // ── User preset scanning ──────────────────────────────────────────────────
    void scanUserPresets();
    static juce::File getUserPresetsDir();
    int  getTotalNumPrograms() const;
    bool saveVstPreset (const juce::File& file);
    bool loadVstPreset (const juce::File& file);

    // ── MIDI Learn system ──────────────────────────────────────────────────────
    static constexpr int kNumLearnableParams = 6;
    static constexpr const char* learnableParamIds[kNumLearnableParams] = {
        "cutoff", "resonance", "envmod", "decay", "accent", "slide"
    };
    std::atomic<int> midiCCMap[kNumLearnableParams] = { {-1},{-1},{-1},{-1},{-1},{-1} };
    std::atomic<int> midiLearnActiveParam { -1 };
    int mlKeyUp  [kNumLearnableParams] { 0,0,0,0,0,0 };
    int mlKeyDown[kNumLearnableParams] { 0,0,0,0,0,0 };

    void startMidiLearn(int paramIndex)  { midiLearnActiveParam.store(paramIndex); }
    void stopMidiLearn()                 { midiLearnActiveParam.store(-1); }
    void clearMidiLearn(int paramIndex)  { midiCCMap[paramIndex].store(-1); }

private:
    std::vector<juce::File> userPresetFiles;

    // ── Note preview atomics ─────────────────────────────────────────────────
    std::atomic<int>   previewNoteOnNote  { -1 };
    std::atomic<float> previewNoteOnVel   { 0.5f };
    std::atomic<int>   previewNoteOffNote { -1 };
    std::atomic<int>   previewNoteSlide   { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SOLAR303Processor)
};
