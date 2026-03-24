#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FactoryPresets.h"

juce::AudioProcessorValueTreeState::ParameterLayout
SOLAR303Processor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // CUTOFF: 50kA log pot → anti-log converter (double non-linearity)
    // skew 0.35 → steeper curve matching TB-303 control law
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"cutoff", 1}, "Cutoff",
        juce::NormalisableRange<float>(60.f, 5000.f, 0.f, 0.35f), 260.07f));

    // RESO: skew 2.0 → exponential — squelch kicks in dramatically near the top
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"resonance", 1}, "Resonance",
        juce::NormalisableRange<float>(0.f, 1.f, 0.f, 2.0f), 1.0f));

    // ENV MOD: skew 0.5 → logarithmic feel, knob centre = 0.25 (more precision at low depths)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"envmod", 1}, "Env Mod",
        juce::NormalisableRange<float>(0.f, 1.f, 0.f, 0.5f), 0.2138f));

    // DECAY: tau range 0.20–2.5s (TB-303 spec)
    // skew 0.35 → 0–30% of knob = most change, above 70% barely changes
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"decay", 1}, "Decay",
        juce::NormalisableRange<float>(0.2f, 2.5f, 0.f, 0.35f), 0.25f));

    // ACCENT: skew 0.5 → quadratic — bark builds dramatically as you open the knob higher
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"accent", 1}, "Accent",
        juce::NormalisableRange<float>(0.f, 1.f, 0.f, 0.5f), 0.4379f));

    // SLIDE: TB-303 spec tau ~50–70ms, starting point 60ms
    // Exponential RC portamento
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"slide", 1}, "Slide",
        juce::NormalisableRange<float>(0.02f, 0.3f, 0.f, 0.4f), 0.0668f));

    // Internal engine gain — fixed at 1.0 (overall output controlled by Master Vol).
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"volume", 1}, "Volume",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"waveform", 1}, "Waveform",
        juce::StringArray{"Sawtooth", "Square"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"tuning", 1}, "Tuning",
        juce::NormalisableRange<float>(-24.f, 24.f, 1.f), 0.f));

    // ── Master volume ─────────────────────────────────────────────────────────
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mastervol", 1}, "Master Vol",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 1.0f));

    // ── FX: Reverb ────────────────────────────────────────────────────────────
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"revon", 1}, "Rev On", false));  // OFF by default
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"revsize", 1}, "Rev Size",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.42f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"revdamp", 1}, "Rev Damp",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"revmix", 1}, "Rev Mix",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.15f));

    // ── FX: Delay ─────────────────────────────────────────────────────────────
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"delayon", 1}, "Delay On", false));  // OFF by default
    // Extended to 2.0 s so note-division presets can cover 4/4 at ≥ 60 BPM
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delaytime", 1}, "Delay Time",
        juce::NormalisableRange<float>(0.02f, 2.0f, 0.001f, 0.4f), 0.250f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayfeed", 1}, "Delay Feedback",
        juce::NormalisableRange<float>(0.f, 0.95f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delaymix", 1}, "Delay Mix",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.21f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"diston", 1}, "Dist On", false));  // OFF by default

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"disttype", 1}, "Dist Type",
        juce::StringArray{"Tanh", "Hard", "Fuzz", "Fold", "Tube", "Crush"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distamount", 1}, "Dist Amount",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distvol", 1}, "Dist Volume",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.7f));

    return { params.begin(), params.end() };
}

SOLAR303Processor::SOLAR303Processor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SOLAR303State", createParameterLayout())
{
    scanUserPresets();
    lastPresetDirMTime = getUserPresetsDir().getLastModificationTime();
    startTimer(2000); // rescan user presets every 2 seconds
}

SOLAR303Processor::~SOLAR303Processor() {}

void SOLAR303Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    engine.prepare(sampleRate);

    // ── Sync BPM from host immediately on prepare ────────────────────────────
    if (auto* ph = getPlayHead())
    {
        if (auto posInfo = ph->getPosition())
        {
            if (auto bpmOpt = posInfo->getBpm())
            {
                if (*bpmOpt > 0.0)
                {
                    sequencer.bpm.store((float)*bpmOpt);
                    bpmSyncedFromHost.store(true);
                }
            }
        }
    }

    // Update sample rate without resetting sequencer state (preserves playing flag)
    sequencer.sr = sampleRate;
    sequencer.pendingReset.store(true);   // audio thread will re-sync position safely

    // ── FX init ───────────────────────────────────────────────────────────────
    const int maxDelay = (int)(sampleRate * 2.0) + 1;
    delayBuf.assign(maxDelay, 0.f);
    delayWritePos = 0;
    reverb.reset();
}

void SOLAR303Processor::releaseResources()
{
    engine.reset();
    // Do NOT call sequencer.stop() here — JUCE Standalone calls releaseResources()
    // during device reconfiguration, which would kill the playing state.
    // Instead, just flag a position reset so the audio thread re-syncs cleanly.
    sequencer.pendingReset.store(true);
}

void SOLAR303Processor::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // ── Host BPM sync — read from DAW every block ─────────────────────────────
    if (auto* ph = getPlayHead())
    {
        if (auto posInfo = ph->getPosition())
        {
            if (auto bpmOpt = posInfo->getBpm())
            {
                if (*bpmOpt > 0.0)
                {
                    sequencer.bpm.store((float)*bpmOpt);
                    bpmSyncedFromHost.store(true);
                }
            }
        }
    }

    // ── Note preview — editor button press/release (lock-free) ───────────────
    {
        const int previewOn = previewNoteOnNote.exchange(-1);
        if (previewOn >= 0)
        {
            const float vel      = previewNoteOnVel.load();
            const bool  doSlide  = previewNoteSlide.exchange(0) != 0;

            if (doSlide)
            {
                // Simulate portamento: first trigger a note 5 semitones below,
                // then immediately slide (legato) to the actual note.
                // Clean up the source note so heldNotes stays consistent.
                const int srcNote = juce::jmax(24, previewOn - 5);
                engine.noteOn (srcNote,   vel);   // 1. freq = srcNote, gate opens
                engine.noteOn (previewOn, vel);   // 2. legato → slide to target
                engine.noteOff(srcNote);          // 3. remove dummy note; engine stays on target
            }
            else
            {
                engine.noteOn(previewOn, vel);
            }
        }

        const int previewOff = previewNoteOffNote.exchange(-1);
        if (previewOff >= 0)
            engine.noteOff(previewOff);
    }

    // ── MIDI Learn: capture CC + apply mapped CCs to parameters ────────────
    for (const auto meta : midiMessages)
    {
        const auto msg = meta.getMessage();
        if (msg.isController())
        {
            const int cc  = msg.getControllerNumber();
            const int val = msg.getControllerValue();

            // If we're in learn mode, assign this CC to the active param
            const int learnParam = midiLearnActiveParam.load();
            if (learnParam >= 0 && learnParam < kNumLearnableParams)
            {
                // Clear any other param that had this CC
                for (int i = 0; i < kNumLearnableParams; ++i)
                    if (midiCCMap[i].load() == cc) midiCCMap[i].store(-1);

                midiCCMap[learnParam].store(cc);
                midiLearnActiveParam.store(-1);  // done learning
            }

            // Apply mapped CCs to parameters
            for (int i = 0; i < kNumLearnableParams; ++i)
            {
                if (midiCCMap[i].load() == cc)
                {
                    if (auto* p = apvts.getParameter(learnableParamIds[i]))
                    {
                        p->beginChangeGesture();
                        p->setValueNotifyingHost(val / 127.f);
                        p->endChangeGesture();
                    }
                }
            }
        }
    }

    // Read synth parameters
    engine.pCutoff     = apvts.getRawParameterValue("cutoff")->load();
    engine.pResonance  = apvts.getRawParameterValue("resonance")->load();
    engine.pEnvMod     = apvts.getRawParameterValue("envmod")->load();
    engine.pDecay      = apvts.getRawParameterValue("decay")->load();
    engine.pAccent     = apvts.getRawParameterValue("accent")->load();
    engine.pSlide      = apvts.getRawParameterValue("slide")->load();
    engine.pVolume     = apvts.getRawParameterValue("volume")->load();
    engine.pSquareWave  = (int)apvts.getRawParameterValue("waveform")->load() == 1;
    engine.pTuning      = apvts.getRawParameterValue("tuning")->load();
    const float masterVol = apvts.getRawParameterValue("mastervol")->load();
    engine.pRevOn     = apvts.getRawParameterValue("revon")->load()    > 0.5f;
    engine.pRevSize   = apvts.getRawParameterValue("revsize")->load();
    engine.pRevDamp   = apvts.getRawParameterValue("revdamp")->load();
    engine.pRevMix    = apvts.getRawParameterValue("revmix")->load();
    engine.pDelayOn   = apvts.getRawParameterValue("delayon")->load() > 0.5f;
    engine.pDelayTime = apvts.getRawParameterValue("delaytime")->load();
    engine.pDelayFeed = apvts.getRawParameterValue("delayfeed")->load();
    engine.pDelayMix  = apvts.getRawParameterValue("delaymix")->load();
    engine.pDistOn      = apvts.getRawParameterValue("diston")->load() > 0.5f;
    engine.pDistType    = (int)apvts.getRawParameterValue("disttype")->load();
    engine.pDistAmount  = apvts.getRawParameterValue("distamount")->load();
    engine.pDistVolume  = apvts.getRawParameterValue("distvol")->load();

    // If sequencer is running, it drives the engine — ignore external MIDI
    const bool isSeqPlaying = sequencer.playing.load();

    // Transition: stopped → playing — reset engine for clean start
    if (isSeqPlaying && !prevSeqPlaying)
        engine.allNotesOff();

    // Transition: playing → stopped — silence any note the sequencer left open
    // Without this, the last played note rings indefinitely (the "buzz after stop" bug)
    if (!isSeqPlaying && prevSeqPlaying)
        engine.allNotesOff();

    prevSeqPlaying = isSeqPlaying;

    if (isSeqPlaying)
    {
        sequencer.process(engine, buffer.getNumSamples());
    }
    else
    {
        // External MIDI
        for (const auto& meta : midiMessages)
        {
            const auto msg = meta.getMessage();
            if (msg.isNoteOn())
                engine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                engine.noteOff(msg.getNoteNumber());
            else if (msg.isAllNotesOff() || msg.isAllSoundOff())
                engine.allNotesOff();
        }
    }

    auto* left  = buffer.getWritePointer(0);
    auto* right = getTotalNumOutputChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float s = engine.processSample();
        // Safety: prevent NaN/Inf from reaching the audio driver
        if (!std::isfinite(s)) s = 0.0f;
        s = juce::jlimit(-1.5f, 1.5f, s);
        left[i]  = s;
        if (right) right[i] = s;
    }

    // ── Delay (applied to full buffer after synthesis) ────────────────────────
    if (engine.pDelayOn && !delayBuf.empty())
    {
        const int bufSize     = (int)delayBuf.size();
        const int delaySamples = std::max(1, (int)(engine.pDelayTime * getSampleRate()));
        const float feed      = std::min(engine.pDelayFeed, 0.95f);
        const float wetMix    = engine.pDelayMix;
        const float dryMix    = 1.0f - wetMix;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const int readPos = ((delayWritePos - delaySamples) % bufSize + bufSize) % bufSize;
            const float wet   = delayBuf[readPos];
            delayBuf[delayWritePos] = left[i] + wet * feed;
            delayWritePos = (delayWritePos + 1) % bufSize;

            const float mixed = left[i] * dryMix + wet * wetMix;
            left[i] = mixed;
            if (right) right[i] = mixed;
        }
    }

    // ── Reverb (applied after delay) ─────────────────────────────────────────
    if (engine.pRevOn)
    {
        // Only update parameters when they actually change — prevents internal
        // state reset in juce::Reverb that would cause clicks every block
        juce::Reverb::Parameters rp;
        rp.roomSize   = engine.pRevSize;
        rp.damping    = engine.pRevDamp;
        rp.wetLevel   = engine.pRevMix;
        rp.dryLevel   = 1.0f;
        rp.width      = 0.85f;
        rp.freezeMode = 0.f;

        if (rp.roomSize   != cachedRevParams.roomSize   ||
            rp.damping    != cachedRevParams.damping    ||
            rp.wetLevel   != cachedRevParams.wetLevel)
        {
            reverb.setParameters(rp);
            cachedRevParams = rp;
        }

        if (right)
            reverb.processStereo(left, right, buffer.getNumSamples());
        else
            reverb.processMono(left, buffer.getNumSamples());
    }

    // ── Master volume (applied last — after all FX) ───────────────────────────
    if (masterVol < 0.999f)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            left[i] *= masterVol;
            if (right) right[i] *= masterVol;
        }
    }

}

void SOLAR303Processor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    // Persist sequencer steps (note, active, accent, slide for each of 16 steps)
    for (int i = 0; i < SOLAR303Sequencer::NUM_STEPS; ++i)
    {
        const auto& s = sequencer.steps[i];
        xml->setAttribute("step_note_"   + juce::String(i), s.note);
        xml->setAttribute("step_active_" + juce::String(i), (int)s.active);
        xml->setAttribute("step_accent_" + juce::String(i), (int)s.accent);
        xml->setAttribute("step_slide_"  + juce::String(i), (int)s.slide);
    }

    // Persist BPM + Length
    xml->setAttribute("bpm", (double)sequencer.bpm.load());
    xml->setAttribute("numSteps", sequencer.numSteps.load());

    // Persist MIDI Learn mappings
    for (int i = 0; i < kNumLearnableParams; ++i)
        xml->setAttribute(juce::String("midicc_") + learnableParamIds[i], midiCCMap[i].load());

    // Persist ML keyboard keys
    for (int i = 0; i < kNumLearnableParams; ++i)
    {
        xml->setAttribute(juce::String("mlkeyup_")   + juce::String(i), mlKeyUp[i]);
        xml->setAttribute(juce::String("mlkeydown_") + juce::String(i), mlKeyDown[i]);
    }

    // Schema version — used to detect old states and apply new FX defaults
    xml->setAttribute("schemaVersion", 2);

    // Persist current preset index so the host shows the right name after reload
    xml->setAttribute("currentProgram", currentProgram);

    copyXmlToBinary(*xml, destData);
}

void SOLAR303Processor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    // Fallback: if JUCE binary parsing failed, try raw XML text (older preset formats)
    if (xml == nullptr)
    {
        juce::String xmlStr(static_cast<const char*>(data), (size_t)sizeInBytes);
        xml = juce::XmlDocument::parse(xmlStr);
    }

    if (xml != nullptr)
    {
        // Accept presets saved under any previous name (TB303State, DS303State, etc.)
        auto loadedTree = juce::ValueTree::fromXml(*xml);
        if (loadedTree.isValid())
        {
            // If the type doesn't match (old preset), rebuild a correctly-typed tree
            if (loadedTree.getType() != apvts.state.getType())
            {
                juce::ValueTree corrected(apvts.state.getType());
                corrected.copyPropertiesFrom(loadedTree, nullptr);
                for (int i = 0; i < loadedTree.getNumChildren(); ++i)
                    corrected.addChild(loadedTree.getChild(i).createCopy(), -1, nullptr);
                apvts.replaceState(corrected);
            }
            else
            {
                apvts.replaceState(loadedTree);
            }
        }

        // If state is old (no schemaVersion), reset FX to new defaults
        if (xml->getIntAttribute("schemaVersion", 1) < 2)
        {
            auto setParam = [&](const char* id, float val)
            {
                if (auto* p = apvts.getParameter(id))
                    p->setValueNotifyingHost(val);
            };
            setParam("revsize",   apvts.getParameter("revsize")  ->convertTo0to1(0.13f));
            setParam("revdamp",   apvts.getParameter("revdamp")  ->convertTo0to1(0.40f));
            setParam("revmix",    apvts.getParameter("revmix")   ->convertTo0to1(0.21f));
            setParam("delaytime", apvts.getParameter("delaytime")->convertTo0to1(0.231f));
            setParam("delayfeed", apvts.getParameter("delayfeed")->convertTo0to1(0.0f));
            setParam("delaymix",  apvts.getParameter("delaymix") ->convertTo0to1(0.13f));
        }

        // Restore sequencer steps
        for (int i = 0; i < SOLAR303Sequencer::NUM_STEPS; ++i)
        {
            auto& s = sequencer.steps[i];
            s.note   = xml->getIntAttribute("step_note_"   + juce::String(i), 48);
            s.active = xml->getIntAttribute("step_active_" + juce::String(i), 1) != 0;
            s.accent = xml->getIntAttribute("step_accent_" + juce::String(i), 0) != 0;
            s.slide  = xml->getIntAttribute("step_slide_"  + juce::String(i), 0) != 0;
        }

        // Restore BPM + Length
        sequencer.bpm.store((float)xml->getDoubleAttribute("bpm", 130.0));
        sequencer.numSteps.store(xml->getIntAttribute("numSteps", 16));

        // Restore MIDI Learn mappings
        for (int i = 0; i < kNumLearnableParams; ++i)
            midiCCMap[i].store(xml->getIntAttribute(juce::String("midicc_") + learnableParamIds[i], -1));

        // Restore ML keyboard keys
        for (int i = 0; i < kNumLearnableParams; ++i)
        {
            mlKeyUp[i]   = xml->getIntAttribute(juce::String("mlkeyup_")   + juce::String(i), 0);
            mlKeyDown[i] = xml->getIntAttribute(juce::String("mlkeydown_") + juce::String(i), 0);
        }

        // Restore current preset index so the host shows the correct preset name
        currentProgram = juce::jlimit(0, getTotalNumPrograms() - 1,
                                      xml->getIntAttribute("currentProgram", 0));

        // Refresh editor step grid + BPM slider if the editor is open
        juce::MessageManager::callAsync([this]()
        {
            if (auto* ed = dynamic_cast<SOLAR303Editor*>(getActiveEditor()))
                ed->refreshStepGrid();
        });
    }
}

juce::AudioProcessorEditor* SOLAR303Processor::createEditor()
{
    return new SOLAR303Editor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SOLAR303Processor();
}

// ── Factory + User Presets ────────────────────────────────────────────────────

int SOLAR303Processor::getTotalNumPrograms() const
{
    return NUM_FACTORY_PRESETS + (int)userPresetFiles.size();
}

juce::File SOLAR303Processor::getUserPresetsDir()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
               .getChildFile("VST3 Presets")
               .getChildFile("SOLAR")
               .getChildFile("SOLAR 303");
}

void SOLAR303Processor::timerCallback()
{
    auto dir = getUserPresetsDir();
    auto mtime = dir.getLastModificationTime();
    if (mtime > lastPresetDirMTime)
    {
        lastPresetDirMTime = mtime;
        scanUserPresets();
        updateHostDisplay();
    }
}

void SOLAR303Processor::scanUserPresets()
{
    userPresetFiles.clear();
    auto dir = getUserPresetsDir();
    if (!dir.isDirectory()) return;
    for (auto& f : dir.findChildFiles(juce::File::findFiles, false,
                                      "*.solar;*.zb303;*.tb303;*.vstpreset"))
        userPresetFiles.push_back(f);
    std::sort(userPresetFiles.begin(), userPresetFiles.end(),
        [](const juce::File& a, const juce::File& b)
        { return a.getFileNameWithoutExtension() < b.getFileNameWithoutExtension(); });
}

// ── Steinberg .vstpreset format writer ────────────────────────────────────────
// Header: 'VST3'(4) + version(4 LE) + ClassID(32 ASCII) + listOffset(8 LE)
// Then: state data, then chunk list 'List'+count+'Comp'+offset+size
bool SOLAR303Processor::saveVstPreset(const juce::File& file)
{
    juce::MemoryBlock state;
    getStateInformation(state);

    const juce::String classID = "ABCDEF019182FAEB536F6C72536F3033";
    const int64_t stateOffset  = 48;                           // after header
    const int64_t stateSize    = (int64_t)state.getSize();
    const int64_t listOffset   = stateOffset + stateSize;

    auto wLE32 = [](juce::MemoryOutputStream& s, int32_t v) {
        s.writeByte((uint8_t)(v));       s.writeByte((uint8_t)(v >> 8));
        s.writeByte((uint8_t)(v >> 16)); s.writeByte((uint8_t)(v >> 24));
    };
    auto wLE64 = [](juce::MemoryOutputStream& s, int64_t v) {
        for (int b = 0; b < 8; ++b) s.writeByte((uint8_t)(v >> (b * 8)));
    };

    juce::MemoryOutputStream out;
    out.writeByte('V'); out.writeByte('S'); out.writeByte('T'); out.writeByte('3'); // magic
    wLE32(out, 1);                                              // version
    out.write(classID.toRawUTF8(), 32);                         // ClassID (32 chars)
    wLE64(out, listOffset);                                     // offset to chunk list
    out.write(state.getData(), state.getSize());                // component state
    // Chunk list
    out.writeByte('L'); out.writeByte('i'); out.writeByte('s'); out.writeByte('t');
    wLE32(out, 1);                                              // 1 chunk
    out.writeByte('C'); out.writeByte('o'); out.writeByte('m'); out.writeByte('p');
    wLE64(out, stateOffset);                                    // offset
    wLE64(out, stateSize);                                      // size

    return file.replaceWithData(out.getData(), out.getDataSize());
}

// ── Load from .vstpreset — searches for JUCE state marker ('VC2!') ────────────
bool SOLAR303Processor::loadVstPreset(const juce::File& file)
{
    juce::MemoryBlock block;
    if (!file.loadFileAsData(block)) return false;

    const auto* raw = static_cast<const char*>(block.getData());
    const int   sz  = (int)block.getSize();

    // Search for JUCE binary XML signature
    for (int i = 0; i < sz - 4; ++i)
    {
        if (raw[i]=='V' && raw[i+1]=='C' && raw[i+2]=='2' && raw[i+3]=='!')
        {
            setStateInformation(raw + i, sz - i);
            return true;
        }
    }
    // Fallback: no header (plain .solar / .zb303)
    setStateInformation(block.getData(), sz);
    return true;
}

int SOLAR303Processor::getNumPrograms()
{
    return getTotalNumPrograms();
}

const juce::String SOLAR303Processor::getProgramName(int index)
{
    if (index >= 0 && index < NUM_FACTORY_PRESETS)
        return FACTORY_PRESETS[index].name;
    const int ui = index - NUM_FACTORY_PRESETS;
    if (ui >= 0 && ui < (int)userPresetFiles.size())
        return userPresetFiles[ui].getFileNameWithoutExtension();
    return "User Preset";
}

void SOLAR303Processor::setCurrentProgram(int index)
{
    const int total = getTotalNumPrograms();
    if (index < 0 || index >= total) return;
    currentProgram = index;

    if (index < NUM_FACTORY_PRESETS)
    {
        loadFactoryPreset(index);
    }
    else
    {
        const int ui = index - NUM_FACTORY_PRESETS;
        if (ui < (int)userPresetFiles.size())
            loadVstPreset(userPresetFiles[ui]);
    }
    updateHostDisplay();
}

void SOLAR303Processor::loadFactoryPreset(int index)
{
    if (index < 0 || index >= NUM_FACTORY_PRESETS) return;
    const auto& p = FACTORY_PRESETS[index];

    // Helper: set a float param by actual value (handles skew automatically)
    auto setFloat = [this](const char* id, float actualValue)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(id)))
            param->setValueNotifyingHost(
                param->getNormalisableRange().convertTo0to1(actualValue));
    };

    // Helper: set a bool param
    auto setBool = [this](const char* id, bool value)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(value ? 1.f : 0.f);
    };

    // ── Synth ────────────────────────────────────────────────────────────────
    setFloat("cutoff",    p.cutoff);
    setFloat("resonance", p.resonance);
    setFloat("envmod",    p.envmod);
    setFloat("decay",     p.decay);
    setFloat("accent",    p.accent);
    setFloat("slide",     p.slide);
    setFloat("volume",    p.volume);
    setFloat("mastervol", p.mastervol);
    setFloat("tuning",    p.tuning);

    // Waveform (choice: 0=Saw, 1=Square)
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("waveform")))
        param->setValueNotifyingHost(p.waveform == 0 ? 0.f : 1.f);

    // ── FX ───────────────────────────────────────────────────────────────────
    setBool ("revon",     p.revOn);
    setFloat("revsize",   p.revSize);
    setFloat("revdamp",   p.revDamp);
    setFloat("revmix",    p.revMix);

    setBool ("delayon",   p.delayOn);
    setFloat("delaytime", p.delayTime);
    setFloat("delayfeed", p.delayFeed);
    setFloat("delaymix",  p.delayMix);

    setBool ("diston",    p.distOn);
    setFloat("distamount",p.distAmount);
    setFloat("distvol",   p.distVol);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("disttype")))
    {
        int numTypes = param->choices.size();
        param->setValueNotifyingHost(numTypes > 1
            ? (float)p.distType / (float)(numTypes - 1) : 0.f);
    }

    // ── Sequencer ─────────────────────────────────────────────────────────────
    sequencer.bpm.store((int)p.bpm);
    sequencer.numSteps.store(p.numSteps);

    for (int i = 0; i < SOLAR303Sequencer::NUM_STEPS; ++i)
    {
        const auto& s = p.steps[i];
        sequencer.steps[i].note   = s.note;
        sequencer.steps[i].active = s.active;
        sequencer.steps[i].accent = s.accent;
        sequencer.steps[i].slide  = s.slide;
    }

    // Guarantee full volume on every preset load
    if (auto* p = apvts.getParameter("volume"))    p->setValueNotifyingHost(1.0f);
    if (auto* p = apvts.getParameter("mastervol")) p->setValueNotifyingHost(1.0f);

    // Refresh editor UI (step grid + knobs)
    juce::MessageManager::callAsync([this]()
    {
        if (auto* ed = dynamic_cast<SOLAR303Editor*>(getActiveEditor()))
            ed->refreshStepGrid();
    });
}
