#pragma once
#include <JuceHeader.h>
#include "SOLAR303Engine.h"
#include "SOLAR303Sequencer.h"

class SOLAR303Processor : public juce::AudioProcessor
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

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

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

    // ── Output High-Pass Filter (Lo-Cut) ─────────────────────────────────────
    std::atomic<float> hpfFreq  { 0.f };
    std::atomic<int>   hpfSlope { 0 };

    void updateHpfCoeffs(float freqHz, int slope);

    struct HpfBiquad {
        float b0=1.f, b1=0.f, b2=0.f;
        float a1=0.f, a2=0.f;
        float x1[2]={0.f,0.f}, x2[2]={0.f,0.f};
        float y1[2]={0.f,0.f}, y2[2]={0.f,0.f};
    };
    HpfBiquad hpfStages[4];
    int       hpfNumStages   = 0;
    float     hpfCurrentFreq = 0.f;
    int       hpfCurrentSlope = -1;

private:
    SOLAR303Engine    engine;
    SOLAR303Sequencer sequencer;
    bool prevSeqPlaying  { false };

    juce::Reverb       reverb;
    std::vector<float> delayBuf;
    int                delayWritePos { 0 };

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<bool>  bpmSyncedFromHost  { false }; // true after first processBlock BPM read

public:
    // ── MIDI Learn system ──────────────────────────────────────────────────────
    // 6 learnable params: cutoff, resonance, envmod, decay, accent, slide
    static constexpr int kNumLearnableParams = 6;
    static constexpr const char* learnableParamIds[kNumLearnableParams] = {
        "cutoff", "resonance", "envmod", "decay", "accent", "slide"
    };

    // CC mapping: -1 = not mapped
    std::atomic<int> midiCCMap[kNumLearnableParams] = { {-1},{-1},{-1},{-1},{-1},{-1} };

    // Which param is currently in "learn" mode (-1 = none)
    std::atomic<int> midiLearnActiveParam { -1 };

    void startMidiLearn(int paramIndex)  { midiLearnActiveParam.store(paramIndex); }
    void stopMidiLearn()                 { midiLearnActiveParam.store(-1); }
    void clearMidiLearn(int paramIndex)  { midiCCMap[paramIndex].store(-1); }

private:

    // ── Note preview atomics ─────────────────────────────────────────────────
    std::atomic<int>   previewNoteOnNote  { -1 };
    std::atomic<float> previewNoteOnVel   { 0.5f };
    std::atomic<int>   previewNoteOffNote { -1 };
    std::atomic<int>   previewNoteSlide   { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SOLAR303Processor)
};
