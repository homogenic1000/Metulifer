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
            juce::ParameterID { id + "note1", 1 }, nm + "Note 1",
            juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "wave1", 1 }, nm + "Wave 1",
            juce::StringArray { "Sine", "Saw", "Square", "Triangle" }, 1));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "octave2", 1 }, nm + "Octave 2", -2, 2, 0));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "note2", 1 }, nm + "Note 2",
            juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
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
    {
        stepsTree.setProperty ("p" + juce::String (i), 0, nullptr);

        voiceSources[i].bind (apvts, i);
        patterns[i].store (getStepPattern (i), std::memory_order_relaxed);
    }

    SequencerEngine::Sources src;
    src.patterns = patterns;
    src.tempo = apvts.getRawParameterValue ("tempo");

    for (int i = 0; i < numSequences; ++i)
        src.lengths[i] = apvts.getRawParameterValue (seqParamId (i, "length"));

    engine.setSources (src);
    stepsTree.addListener (this);
}

MetuliferAudioProcessor::~MetuliferAudioProcessor()
{
    stepsTree.removeListener (this);
}

//==============================================================================
void MetuliferAudioProcessor::VoiceSources::bind (juce::AudioProcessorValueTreeState& apvts, int sequenceIndex)
{
    const auto raw = [&apvts, sequenceIndex] (const char* name)
    {
        return apvts.getRawParameterValue (MetuliferAudioProcessor::seqParamId (sequenceIndex, name));
    };

    a1     = raw ("a1");     d1     = raw ("d1");
    s1     = raw ("s1");     r1     = raw ("r1");
    a2     = raw ("a2");     d2     = raw ("d2");
    s2     = raw ("s2");     r2     = raw ("r2");
    octave1 = raw ("octave1"); note1 = raw ("note1"); wave1 = raw ("wave1");
    octave2 = raw ("octave2"); note2 = raw ("note2"); wave2 = raw ("wave2");
    mix1   = raw ("mix1");    mix2   = raw ("mix2");
    filter = raw ("filter");  cutoff = raw ("cutoff");
    volume = raw ("volume");
}

Voice::Params MetuliferAudioProcessor::VoiceSources::read() const
{
    const auto v = [] (const std::atomic<float>* p) { return p != nullptr ? p->load (std::memory_order_relaxed) : 0.0f; };

    Voice::Params p;
    p.attack1   = v (a1);      p.decay1   = v (d1);
    p.sustain1  = v (s1);      p.release1 = v (r1);
    p.attack2   = v (a2);      p.decay2   = v (d2);
    p.sustain2  = v (s2);      p.release2 = v (r2);
    p.octave1   = (int) v (octave1);  p.note1 = (int) v (note1);  p.wave1 = (int) v (wave1);
    p.octave2   = (int) v (octave2);  p.note2 = (int) v (note2);  p.wave2 = (int) v (wave2);
    p.mixDb1    = v (mix1);    p.mixDb2   = v (mix2);
    p.useLpf    = v (filter) < 0.5f;
    p.cutoff    = v (cutoff);
    p.volumePct = v (volume);
    return p;
}

void MetuliferAudioProcessor::setPlaying (bool shouldPlay)
{
    engine.setPlaying (shouldPlay);
}

bool MetuliferAudioProcessor::isPlaying() const
{
    return engine.isPlaying();
}

int MetuliferAudioProcessor::getPlayheadStep (int sequenceIndex) const
{
    return engine.getCurrentStep (sequenceIndex);
}

void MetuliferAudioProcessor::valueTreePropertyChanged (juce::ValueTree& tree,
                                                        const juce::Identifier& property)
{
    juce::ignoreUnused (property);

    if (tree == stepsTree)
    {
        for (int i = 0; i < numSequences; ++i)
            patterns[i].store (getStepPattern (i), std::memory_order_relaxed);
    }
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
    return 6.0;
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
    engine.prepare (sampleRate);

    for (auto& voice : voices)
        voice.prepare (sampleRate, samplesPerBlock);
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

void MetuliferAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    for (int s = 0; s < numSequences; ++s)
        voices[s].setParams (voiceSources[s].read());

    if (engine.takePendingNoteOff())
        for (auto& voice : voices)
            voice.noteOff();

    const auto& events = engine.process (numSamples);

    int pos = 0;

    for (const auto& ev : events)
    {
        if (ev.sampleOffset > pos)
            renderVoices (buffer, pos, ev.sampleOffset - pos);

        auto& voice = voices[ev.sequence];

        if (ev.gateOn)
            voice.noteOn();
        else
            voice.noteOff();

        pos = ev.sampleOffset;
    }

    if (pos < numSamples)
        renderVoices (buffer, pos, numSamples - pos);

    // Output RMS for the display screen (lock-free, realtime safe).
    {
        float sumSquares = 0.0f;
        int count = 0;

        for (int channel = 0; channel < numChannels; ++channel)
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

void MetuliferAudioProcessor::renderVoices (juce::AudioBuffer<float>& buffer,
                                            int startSample, int numSamples)
{
    for (auto& voice : voices)
        voice.render (buffer, startSample, numSamples);
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
    {
        stepsTree.copyPropertiesAndChildrenFrom (stepsState, nullptr);

        for (int i = 0; i < numSequences; ++i)
            patterns[i].store (getStepPattern (i), std::memory_order_relaxed);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetuliferAudioProcessor();
}
