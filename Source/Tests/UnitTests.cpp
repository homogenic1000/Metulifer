#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../Sequencer/SequencerEngine.h"
#include "../DSP/Voice.h"

//==============================================================================
class SequencerUnitTest : public juce::UnitTest
{
public:
    SequencerUnitTest() : juce::UnitTest ("Sequencer", "Core") {}

    void runTest() override
    {
        beginTest ("Playback timing and step advancement");
        {
            SequencerEngine engine;
            engine.prepare (44100.0);

            std::atomic<int> patterns[6] {};
            std::atomic<float> lengths[6] {};
            std::atomic<float> tempo { 120.0f };

            // Set track 1 length to 16, track 2 length to 8
            lengths[0].store (16.0f);
            lengths[1].store (8.0f);
            for (int i = 2; i < 6; ++i)
                lengths[i].store (32.0f);

            // Pattern with step 0 active on track 0
            patterns[0].store (1); // bit 0 = 1

            SequencerEngine::Sources s;
            s.patterns = patterns;
            for (int i = 0; i < 6; ++i)
                s.lengths[i] = &lengths[i];
            s.tempo = &tempo;

            engine.setSources (s);
            engine.setPlaying (true);

            expect (engine.isPlaying());

            // Process one full block (512 samples)
            const auto& events = engine.process (512);

            // Should trigger step 0 event
            bool foundGateOn = false;
            for (const auto& ev : events)
            {
                if (ev.sequence == 0 && ev.gateOn)
                    foundGateOn = true;
            }
            expect (foundGateOn, "Expected gateOn for sequence 0 at step 0");

            engine.setPlaying (false);
            expect (! engine.isPlaying());
            expect (engine.takePendingNoteOff());
        }
    }
};

static SequencerUnitTest sequencerUnitTest;

//==============================================================================
class HostSyncUnitTest : public juce::UnitTest
{
public:
    HostSyncUnitTest() : juce::UnitTest ("Host Transport & BPM Sync", "DAW") {}

    void runTest() override
    {
        beginTest ("DAW Timeline and Transport Synchronization");
        {
            SequencerEngine engine;
            engine.prepare (44100.0);

            std::atomic<int> patterns[6] {};
            std::atomic<float> lengths[6] {};
            std::atomic<float> tempo { 120.0f };
            for (int i = 0; i < 6; ++i)
                lengths[i].store (16.0f);

            SequencerEngine::Sources s;
            s.patterns = patterns;
            for (int i = 0; i < 6; ++i)
                s.lengths[i] = &lengths[i];
            s.tempo = &tempo;
            engine.setSources (s);

            expect (! engine.isPlaying());

            // 1. Host starts playback at 135 BPM, 1st beat of bar 2 (PPQ = 4.0)
            engine.syncToHost (4.0, 135.0, true);
            expect (engine.isPlaying(), "Engine should start playing when host starts");

            // 2. Host stops
            engine.syncToHost (8.0, 135.0, false);
            expect (! engine.isPlaying(), "Engine should stop when host stops");
            expect (engine.takePendingNoteOff(), "Pending note off must trigger on stop");
        }
    }
};

static HostSyncUnitTest hostSyncUnitTest;

//==============================================================================
class MidiInOutUnitTest : public juce::UnitTest
{
public:
    MidiInOutUnitTest() : juce::UnitTest ("MIDI In & MIDI Out", "MIDI") {}

    void runTest() override
    {
        beginTest ("Incoming MIDI transposition and Outgoing Step MIDI generation");
        {
            MetuliferAudioProcessor proc;
            proc.prepareToPlay (44100.0, 512);

            // 1. Send MIDI Note On 62 (D4, transpose +2 relative to C4=60)
            juce::MidiBuffer inMidi;
            inMidi.addEvent (juce::MidiMessage::noteOn (1, 62, 0.8f), 0);

            juce::AudioBuffer<float> buffer (2, 512);
            buffer.clear();

            proc.processBlock (buffer, inMidi);
            expectEquals (proc.getMidiTranspose(), 2);

            // 2. Set step 0 active on sequence 0 and start playback
            proc.setStepPattern (0, 1);
            proc.setHostSyncEnabled (false); // use internal clock
            proc.setPlaying (true);

            juce::MidiBuffer outMidi;
            proc.processBlock (buffer, outMidi);

            bool generatedNoteOn = false;
            for (const auto meta : outMidi)
            {
                const auto msg = meta.getMessage();
                if (msg.isNoteOn() && msg.getChannel() == 1)
                    generatedNoteOn = true;
            }
            expect (generatedNoteOn, "Sequencer must generate outgoing MIDI noteOn on channel 1");

            proc.setPlaying (false);
        }
    }
};

static MidiInOutUnitTest midiInOutUnitTest;

//==============================================================================
class FactoryPresetsUnitTest : public juce::UnitTest
{
public:
    FactoryPresetsUnitTest() : juce::UnitTest ("Factory Presets", "Presets") {}

    void runTest() override
    {
        beginTest ("Preset loading and program integrity");
        {
            MetuliferAudioProcessor proc;
            expectEquals (proc.getNumPrograms(), 8);

            for (int i = 0; i < 8; ++i)
            {
                proc.setCurrentProgram (i);
                expectEquals (proc.getCurrentProgram(), i);
                expect (proc.getProgramName (i).isNotEmpty(), "Preset name must not be empty");

                // Verify that patterns or parameters exist for this preset
                int patternSeq0 = proc.getStepPattern (0);
                expect (patternSeq0 != 0, "Preset seq 0 pattern should have active steps");
            }
        }
    }
};

static FactoryPresetsUnitTest factoryPresetsUnitTest;

//==============================================================================
class CreativeToolsUnitTest : public juce::UnitTest
{
public:
    CreativeToolsUnitTest() : juce::UnitTest ("Creative Sequencer Tools", "Sequencer") {}

    void runTest() override
    {
        beginTest ("Step shift, euclidean rhythm and randomizer");
        {
            MetuliferAudioProcessor proc;

            // 1. Test Shift: pattern with bit 0 active (0b0001) shifted by 2 becomes 0b0100 (4)
            proc.setStepPattern (0, 1);
            proc.shiftSequence (0, 2);
            expectEquals (proc.getStepPattern (0), 4);

            // Shift backward by 1 becomes 0b0010 (2)
            proc.shiftSequence (0, -1);
            expectEquals (proc.getStepPattern (0), 2);

            // 2. Test Euclidean: 4 hits over 16 steps should place pulses on steps 0, 4, 8, 12
            proc.euclideanSequence (1, 4, 16);
            int euclidPat = proc.getStepPattern (1);
            expect ((euclidPat & (1 << 0)) != 0);
            expect ((euclidPat & (1 << 4)) != 0);
            expect ((euclidPat & (1 << 8)) != 0);
            expect ((euclidPat & (1 << 12)) != 0);

            // 3. Test Randomize
            proc.randomizeSequence (2, 0.5f);
            int randPat = proc.getStepPattern (2);
            expect (randPat != 0, "Randomized pattern should not be empty with 0.5 density");
        }
    }
};

static CreativeToolsUnitTest creativeToolsUnitTest;

//==============================================================================
class DSPSafetyUnitTest : public juce::UnitTest
{
public:
    DSPSafetyUnitTest() : juce::UnitTest ("DSP Safety & Bounds", "Audio") {}

    void runTest() override
    {
        beginTest ("Voice rendering with extreme values and no NaNs");
        {
            Voice voice;
            voice.prepare (44100.0, 512);

            Voice::Params p;
            p.attack1 = 0.1f; p.decay1 = 10.0f; p.sustain1 = 0.0f; p.release1 = 10.0f;
            p.mixDb1 = 0.0f;  p.mixDb2 = 0.0f;
            p.cutoff = 20.0f; // Extreme low cutoff
            p.volumePct = 100.0f;
            p.delayEnabled = true;
            p.delayFeedback = 85.0f;
            p.delayMix = 50.0f;
            p.reverbEnabled = true;
            p.reverbMix = 80.0f;

            voice.setParams (p);
            voice.noteOn();

            juce::AudioBuffer<float> buffer (2, 512);
            buffer.clear();

            for (int block = 0; block < 100; ++block)
            {
                if (block == 10)
                    voice.noteOff();

                voice.render (buffer, 0, 512);

                for (int ch = 0; ch < 2; ++ch)
                {
                    const float* r = buffer.getReadPointer (ch);
                    for (int i = 0; i < 512; ++i)
                    {
                        expect (! std::isnan (r[i]), "Buffer sample is NaN!");
                        expect (! std::isinf (r[i]), "Buffer sample is Inf!");
                    }
                }
            }
        }
    }
};

static DSPSafetyUnitTest dspSafetyUnitTest;

//==============================================================================
class StateSerializationUnitTest : public juce::UnitTest
{
public:
    StateSerializationUnitTest() : juce::UnitTest ("State Serialization", "Core") {}

    void runTest() override
    {
        beginTest ("State save and restore fidelity");
        {
            MetuliferAudioProcessor proc1;
            proc1.setStepPattern (0, 0b10101010);
            proc1.setStepPattern (5, 0b11001100);

            // Modify a parameter
            auto* tempoParam = proc1.apvts.getParameter ("tempo");
            expect (tempoParam != nullptr);
            tempoParam->setValueNotifyingHost (tempoParam->convertTo0to1 (142.0f));

            // Save state
            juce::MemoryBlock state;
            proc1.getStateInformation (state);
            expect (state.getSize() > 0);

            // Restore in another instance
            MetuliferAudioProcessor proc2;
            proc2.setStateInformation (state.getData(), (int) state.getSize());

            expectEquals (proc2.getStepPattern (0), 0b10101010);
            expectEquals (proc2.getStepPattern (5), 0b11001100);

            auto* tempoParam2 = proc2.apvts.getRawParameterValue ("tempo");
            expect (tempoParam2 != nullptr);
            expectWithinAbsoluteError (tempoParam2->load(), 142.0f, 0.5f);
        }
    }
};

static StateSerializationUnitTest stateSerializationUnitTest;
