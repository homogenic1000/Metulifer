/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Sequencer/SequencerEngine.h"
#include "DSP/Voice.h"

//==============================================================================
/**
*/
class MetuliferAudioProcessor  : public juce::AudioProcessor,
                                 private juce::ValueTree::Listener
{
public:
    //==============================================================================
    MetuliferAudioProcessor();
    ~MetuliferAudioProcessor() override;

    static constexpr int numSequences = 6;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    /** "seq1_..." (seqIndex is 0-based, param ids are 1-based). */
    static juce::String seqParamId (int seqIndex, const juce::String& name);

    /** Steps tree: props "p0".."p5", one 32-bit pattern per sequence. */
    juce::ValueTree getStepsTree() const { return stepsTree; }
    int  getStepPattern (int seqIndex) const;
    void setStepPattern (int seqIndex, int pattern);

    float getCurrentRms() const noexcept { return rms.load (std::memory_order_relaxed); }

    void setPlaying (bool shouldPlay);
    bool isPlaying() const;
    int  getPlayheadStep (int sequenceIndex) const;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    struct VoiceSources
    {
        std::atomic<float>* a1 = nullptr;      std::atomic<float>* d1 = nullptr;
        std::atomic<float>* s1 = nullptr;      std::atomic<float>* r1 = nullptr;
        std::atomic<float>* a2 = nullptr;      std::atomic<float>* d2 = nullptr;
        std::atomic<float>* s2 = nullptr;      std::atomic<float>* r2 = nullptr;
        std::atomic<float>* octave1 = nullptr; std::atomic<float>* note1 = nullptr;
        std::atomic<float>* wave1 = nullptr;   std::atomic<float>* octave2 = nullptr;
        std::atomic<float>* note2 = nullptr;   std::atomic<float>* wave2 = nullptr;
        std::atomic<float>* mix1 = nullptr;    std::atomic<float>* mix2 = nullptr;
        std::atomic<float>* filter = nullptr;  std::atomic<float>* cutoff = nullptr;
        std::atomic<float>* volume = nullptr;

        void bind (juce::AudioProcessorValueTreeState& apvts, int sequenceIndex);
        Voice::Params read() const;
    };

    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override;

    juce::ValueTree stepsTree { "steps" };
    std::atomic<float> rms { 0.0f };

    SequencerEngine engine;
    Voice voices[numSequences];
    VoiceSources voiceSources[numSequences];
    std::atomic<int> patterns[numSequences] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetuliferAudioProcessor)
};
