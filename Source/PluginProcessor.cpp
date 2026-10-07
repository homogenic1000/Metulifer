/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
static juce::NormalisableRange<float> msRange()
{
    juce::NormalisableRange<float> range (0.0f, 5000.0f, 0.1f);
    range.setSkewForCentre (500.0f);
    return range;
}

static juce::NormalisableRange<float> cutoffRange()
{
    juce::NormalisableRange<float> range (20.0f, 20000.0f, 1.0f);
    range.setSkewForCentre (1000.0f);
    return range;
}

juce::AudioProcessorValueTreeState::ParameterLayout MetuliferAudioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto msAttr = juce::AudioParameterFloatAttributes()
        .withLabel ("ms")
        .withStringFromValueFunction ([] (float v, int)
        {
            return v < 1.0f ? juce::String (v, 1) : juce::String (juce::roundToInt (v));
        });
    const auto pctAttr  = juce::AudioParameterFloatAttributes().withLabel ("%");
    const auto dbAttr   = juce::AudioParameterFloatAttributes().withLabel ("dB");
    const auto hzAttr   = juce::AudioParameterFloatAttributes().withLabel ("Hz");
    const auto stepAttr = juce::AudioParameterIntAttributes().withLabel ("steps");
    const auto bpmAttr  = juce::AudioParameterIntAttributes().withLabel ("BPM");

    const auto ms = msRange();
    const auto hz = cutoffRange();

    for (int s = 1; s <= numSequences; ++s)
    {
        const juce::String id = "seq" + juce::String (s) + "_";
        const juce::String nm = "Seq" + juce::String (s) + " ";

        for (int v = 1; v <= 2; ++v)
        {
            const auto sv = juce::String (v);

            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { id + "a" + sv, 1 }, nm + "Attack " + sv, ms, 5.0f, msAttr));
            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { id + "d" + sv, 1 }, nm + "Decay " + sv, ms, 200.0f, msAttr));
            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { id + "s" + sv, 1 }, nm + "Sustain " + sv,
                juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 70.0f, pctAttr));
            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { id + "r" + sv, 1 }, nm + "Release " + sv, ms, 300.0f, msAttr));
        }

        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "octave1", 1 }, nm + "Octave 1", -2, 2, 0));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "wave1", 1 }, nm + "Wave 1",
            juce::StringArray { "Sine", "Saw", "Square", "Triangle" }, 1));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "octave2", 1 }, nm + "Octave 2", -2, 2, 0));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "wave2", 1 }, nm + "Wave 2",
            juce::StringArray { "Sine", "Saw", "Square", "Triangle" }, 1));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "mix1", 1 }, nm + "Mix VCO1",
            juce::NormalisableRange<float> (-60.0f, 6.0f, 0.1f), 0.0f, dbAttr));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "mix2", 1 }, nm + "Mix VCO2",
            juce::NormalisableRange<float> (-60.0f, 6.0f, 0.1f), 0.0f, dbAttr));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "filter", 1 }, nm + "Filter",
            juce::StringArray { "LPF", "HPF" }, 0));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "cutoff", 1 }, nm + "Cutoff", hz, 1000.0f, hzAttr));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "length", 1 }, nm + "Length", 1, 32, 32, stepAttr));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "volume", 1 }, nm + "Volume",
            juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 80.0f, pctAttr));
    }

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "tempo", 1 }, "Tempo", 20, 300, 120, bpmAttr));

    return layout;
}

MetuliferAudioProcessor::MetuliferAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
       apvts (*this, &undoManager, "APVTS", createLayout())
{
    for (int i = 0; i < numSequences; ++i)
        stepsTree.setProperty ("p" + juce::String (i), 0, nullptr);
}

MetuliferAudioProcessor::~MetuliferAudioProcessor()
{
}

//==============================================================================
const juce::String MetuliferAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MetuliferAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MetuliferAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MetuliferAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MetuliferAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MetuliferAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MetuliferAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MetuliferAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MetuliferAudioProcessor::getProgramName (int index)
{
    return {};
}

void MetuliferAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
juce::String MetuliferAudioProcessor::seqParamId (int seqIndex, const juce::String& name)
{
    return "seq" + juce::String (seqIndex + 1) + "_" + name;
}

int MetuliferAudioProcessor::getStepPattern (int seqIndex) const
{
    return (int) stepsTree.getProperty ("p" + juce::String (seqIndex), 0);
}

void MetuliferAudioProcessor::setStepPattern (int seqIndex, int pattern)
{
    stepsTree.setProperty ("p" + juce::String (seqIndex), pattern, nullptr);
}

//==============================================================================
void MetuliferAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MetuliferAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MetuliferAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MetuliferAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }

    // Output RMS for the display screen (lock-free, realtime safe).
    {
        const int numSamples = buffer.getNumSamples();
        float sumSquares = 0.0f;
        int count = 0;

        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            const auto* data = buffer.getReadPointer (channel);
            for (int i = 0; i < numSamples; ++i)
                sumSquares += data[i] * data[i];
            count += numSamples;
        }

        rms.store (count > 0 ? std::sqrt (sumSquares / (float) count) : 0.0f,
                   std::memory_order_relaxed);
    }
}

//==============================================================================
bool MetuliferAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MetuliferAudioProcessor::createEditor()
{
    return new MetuliferAudioProcessorEditor (*this);
}

//==============================================================================
void MetuliferAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto root = juce::ValueTree ("MetuliferState");
    root.appendChild (apvts.copyState(), nullptr);
    root.appendChild (stepsTree.createCopy(), nullptr);

    if (auto xml = root.createXml())
        copyXmlToBinary (*xml, destData);
}

void MetuliferAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr)
        return;

    auto root = juce::ValueTree::fromXml (*xml);
    if (! root.isValid())
        return;

    auto paramsState = root.getChildWithName (apvts.copyState().getType());
    if (paramsState.isValid())
        apvts.replaceState (paramsState);

    auto stepsState = root.getChildWithName (stepsTree.getType());
    if (stepsState.isValid())
        stepsTree.copyPropertiesAndChildrenFrom (stepsState, nullptr);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetuliferAudioProcessor();
}
