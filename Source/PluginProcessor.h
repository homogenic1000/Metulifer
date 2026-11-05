/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Sequencer/SequencerEngine.h"
#include "DSP/Voice.h"
#include "DSP/ModMatrix.h"

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

    /** Modulation tree: children cable { src (0|1), dest (param id) }. */
    juce::ValueTree getModTree() const { return modTree; }

    float getCurrentRms() const noexcept { return rms.load (std::memory_order_relaxed); }

    float getLfoValue (int index) const noexcept
    {
        return juce::isPositiveAndBelow (index, 2) ? lfoValue[index].load (std::memory_order_relaxed) : 0.0f;
    }

    float getPeakDbL() const noexcept { return peakDbL.load (std::memory_order_relaxed); }
    float getPeakDbR() const noexcept { return peakDbR.load (std::memory_order_relaxed); }
    float getRmsDbL()  const noexcept { return rmsDbL.load  (std::memory_order_relaxed); }
    float getRmsDbR()  const noexcept { return rmsDbR.load  (std::memory_order_relaxed); }

    static constexpr int scopeBufferSize = 1024;
    void getScopeBuffer (float* dest, int numSamplesToRead) const;

    void setPlaying (bool shouldPlay);
    bool isPlaying() const;
    int  getPlayheadStep (int sequenceIndex) const;

    //==============================================================================
    // Host transport sync & MIDI
    bool isHostSyncEnabled() const;
    void setHostSyncEnabled (bool enabled);
    int  getMidiTranspose() const noexcept { return midiTranspose.load (std::memory_order_relaxed); }
    void setMidiTranspose (int semitones) noexcept { midiTranspose.store (semitones, std::memory_order_relaxed); }

    //==============================================================================
    // Creative Sequencer Tools
    void shiftSequence (int seqIndex, int deltaSteps);
    void randomizeSequence (int seqIndex, float density);
    void euclideanSequence (int seqIndex, int hits, int length);

    //==============================================================================
    // Mute & Solo (Electribe style)
    bool isSequenceMuted (int seqIndex) const;
    void setSequenceMuted (int seqIndex, bool mute);
    bool isSequenceSolo (int seqIndex) const;
    void setSequenceSolo (int seqIndex, bool solo);

    //==============================================================================
    // Modulation Matrix (Ableton style)
    float getMatrixAmount (int seqIndex, int dest, int src) const noexcept;
    void  setMatrixAmount (int seqIndex, int dest, int src, float amount);
    juce::ValueTree& getMatrixTree() noexcept { return matrixTree; }

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
        std::atomic<float>* reverbEnabled = nullptr;
        std::atomic<float>* reverbMode = nullptr;
        std::atomic<float>* reverbMix = nullptr;
        std::atomic<float>* delayEnabled = nullptr;
        std::atomic<float>* delayTime = nullptr;
        std::atomic<float>* delayFeedback = nullptr;
        std::atomic<float>* delayMix = nullptr;
        std::atomic<float>* delayPingpong = nullptr;
        std::atomic<float>* reduxEnabled = nullptr;
        std::atomic<float>* reduxBits = nullptr;
        std::atomic<float>* reduxRate = nullptr;
        std::atomic<float>* reduxMix = nullptr;
        std::atomic<float>* arpEnabled = nullptr;
        std::atomic<float>* arpMode = nullptr;
        std::atomic<float>* arpChord = nullptr;
        std::atomic<float>* arpOctaves = nullptr;
        std::atomic<float>* mute = nullptr;
        std::atomic<float>* solo = nullptr;

        void bind (juce::AudioProcessorValueTreeState& apvts, int sequenceIndex);
        Voice::Params read() const;
    };

    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree& parent, juce::ValueTree&) override;
    void valueTreeChildRemoved (juce::ValueTree& parent, juce::ValueTree&, int) override;

    enum DestField
    {
        fA1, fD1, fS1, fR1, fA2, fD2, fS2, fR2, fMix1, fMix2, fCutoff,
        fReverbMix, fDelayTime, fDelayFeedback, fDelayMix, fReduxBits, fReduxRate, fReduxMix,
        fInvalid
    };

    static DestField fieldForDest (const juce::String& paramId);
    static void setField (Voice::Params& p, DestField field, float value);
    void rebuildCables();
    void updateLfos (int numSamples);

    static constexpr int maxCables = 32;

    struct RuntimeCable
    {
        int lfo = 0;
        int seqIndex = -1;
        DestField field = fInvalid;
        const juce::RangedAudioParameter* param = nullptr;
        const std::atomic<float>* raw = nullptr;
    };

    RuntimeCable cableBuffers[2][maxCables] {};
    std::atomic<int> cableCounts[2] { 0, 0 };
    std::atomic<int> activeBuffer { 0 };

    juce::ValueTree stepsTree { "steps" };
    juce::ValueTree modTree { "mod" };
    juce::ValueTree matrixTree { "matrix" };
    std::atomic<float> modMatrix[numSequences][ModMatrix::numDestinations][ModMatrix::numSources] {};
    std::atomic<float> rms { 0.0f };
    std::atomic<float> peakDbL { -80.0f };
    std::atomic<float> peakDbR { -80.0f };
    std::atomic<float> rmsDbL  { -80.0f };
    std::atomic<float> rmsDbR  { -80.0f };
    float scopeBuffer[scopeBufferSize] {};
    std::atomic<int> scopeWriteIndex { 0 };

    SequencerEngine engine;
    Voice voices[numSequences];
    VoiceSources voiceSources[numSequences];
    std::atomic<int> patterns[numSequences] {};

    // LFO state (message/audio safe: atomics read per block).
    std::atomic<float>* lfoRate[2] {};
    std::atomic<float>* lfoWave[2] {};
    std::atomic<float>* lfoOffset[2] {};
    std::atomic<float>* lfoDepth[2] {};
    std::atomic<float>* lfoEnabled[2] {};
    double lfoPhase[2] {};
    std::atomic<float> lfoValue[2];
    double sampleRate = 44100.0;

    std::atomic<float>* hostSyncParam = nullptr;
    std::atomic<int> midiTranspose { 0 };
    int activeMidiNotes[numSequences] { -1, -1, -1, -1, -1, -1 };
    int currentProgram { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetuliferAudioProcessor)
};
