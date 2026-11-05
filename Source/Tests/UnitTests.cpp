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
