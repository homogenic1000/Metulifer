#include "Test_Sequencer.h"
#include "../Sequencer/SequencerEngine.h"
#include "../PluginProcessor.h"

class Test_Sequencer : public juce::UnitTest
{
public:
    Test_Sequencer() : juce::UnitTest ("Sequencer", "Core") {}

    void runTest() override
    {
        beginTest ("samplesPerStep BPM 120");
        {
            MetuliferAudioProcessor proc;
            SequencerEngine eng;
            SequencerEngine::Sources s {};
            s.tempo = proc.getTempoRawPtr();
            for (int i = 0; i < SequencerEngine::numSequences; ++i)
            {
                s.lengths[i] = nullptr;
                s.volumes[i] = nullptr;
            }
            s.patterns = nullptr;
            eng.setSources (s);
            eng.prepare (44100.0);
            auto v = eng.samplesPerStep();
            expect (v >= 5510.0 && v <= 5520.0);
        }
    }
};

static Test_Sequencer testSequencer;
