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
            juce::ParameterID { id + "cutoff", 1 }, nm + "Cutoff", hz, 20000.0f, hzAttr));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "length", 1 }, nm + "Length", 1, 32, 32, stepAttr));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "volume", 1 }, nm + "Volume",
            juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 80.0f, pctAttr));
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id + "reverb_enabled", 1 }, nm + "Reverb Enabled", false));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "reverb_mode", 1 }, nm + "Reverb Mode",
            juce::StringArray { "Room", "Hall" }, 0));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "reverb_mix", 1 }, nm + "Reverb Mix",
            juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 30.0f, pctAttr));

        // Delay
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id + "delay_enabled", 1 }, nm + "Delay Enabled", false));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "delay_time", 1 }, nm + "Delay Time",
            juce::NormalisableRange<float> (20.0f, 1000.0f, 1.0f, 0.4f), 250.0f, msAttr));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "delay_feedback", 1 }, nm + "Delay Feedback",
            juce::NormalisableRange<float> (0.0f, 90.0f, 1.0f), 40.0f, pctAttr));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "delay_mix", 1 }, nm + "Delay Mix",
            juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 35.0f, pctAttr));
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id + "delay_pingpong", 1 }, nm + "Delay PingPong", true));

        // Redux
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id + "redux_enabled", 1 }, nm + "Redux Enabled", false));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "redux_bits", 1 }, nm + "Redux Bits", 2, 16, 8));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "redux_rate", 1 }, nm + "Redux Rate", 1, 32, 4));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id + "redux_mix", 1 }, nm + "Redux Mix",
            juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 50.0f, pctAttr));

        // Arpeggiator
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id + "arp_enabled", 1 }, nm + "Arp Enabled", false));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "arp_mode", 1 }, nm + "Arp Mode",
            juce::StringArray { "Up", "Down", "Up&Down", "Down&Up", "Pinky Up", "Pinky Down",
                                "Thumb-Pinky", "Converge", "Diverge", "Random", "Order" }, 0));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id + "arp_chord", 1 }, nm + "Arp Chord",
            juce::StringArray { "Major", "Minor", "Pentatonic", "Octaves", "Maj7", "Min7",
                                "Dom7", "Sus4", "Fifths", "Chromatic" }, 1));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id + "arp_octaves", 1 }, nm + "Arp Octaves", 1, 3, 1));
    }

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "tempo", 1 }, "Tempo", 20, 300, 120, bpmAttr));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "host_sync", 1 }, "Host Sync", true));

    {
        juce::NormalisableRange<float> rateRange (0.1f, 20.0f, 0.01f);
        rateRange.setSkewForCentre (1.0f);

        for (int i = 1; i <= 2; ++i)
        {
            const juce::String lid = "lfo" + juce::String (i) + "_";
            const juce::String lnm = "LFO" + juce::String (i) + " ";

            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { lid + "rate", 1 }, lnm + "Rate", rateRange, 1.0f, hzAttr));
            layout.add (std::make_unique<juce::AudioParameterChoice> (
                juce::ParameterID { lid + "wave", 1 }, lnm + "Wave",
                juce::StringArray { "Sine", "Saw", "Square", "Triangle" }, 0));
            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { lid + "offset", 1 }, lnm + "Offset",
                juce::NormalisableRange<float> (-100.0f, 100.0f, 1.0f), 0.0f, pctAttr));
            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID { lid + "depth", 1 }, lnm + "Depth",
                juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f, pctAttr));
            layout.add (std::make_unique<juce::AudioParameterBool> (
                juce::ParameterID { lid + "enabled", 1 }, lnm + "Enabled", false));
        }
    }

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
    }

    SequencerEngine::Sources src;
    src.patterns = patterns;
    src.tempo = apvts.getRawParameterValue ("tempo");

    for (int i = 0; i < numSequences; ++i)
        src.lengths[i] = apvts.getRawParameterValue (seqParamId (i, "length"));

    engine.setSources (src);

    for (int i = 0; i < 2; ++i)
    {
        const auto lid = "lfo" + juce::String (i + 1) + "_";
        lfoRate[i]     = apvts.getRawParameterValue (lid + "rate");
        lfoWave[i]     = apvts.getRawParameterValue (lid + "wave");
        lfoOffset[i]   = apvts.getRawParameterValue (lid + "offset");
        lfoDepth[i]     = apvts.getRawParameterValue (lid + "depth");
        lfoEnabled[i]  = apvts.getRawParameterValue (lid + "enabled");
        lfoValue[i].store (0.0f, std::memory_order_relaxed);
    }

    hostSyncParam = apvts.getRawParameterValue ("host_sync");

    stepsTree.addListener (this);
    modTree.addListener (this);
}

MetuliferAudioProcessor::~MetuliferAudioProcessor()
{
    stepsTree.removeListener (this);
    modTree.removeListener (this);
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
    reverbEnabled = raw ("reverb_enabled");
    reverbMode    = raw ("reverb_mode");
    reverbMix     = raw ("reverb_mix");
    delayEnabled  = raw ("delay_enabled");
    delayTime     = raw ("delay_time");
    delayFeedback = raw ("delay_feedback");
    delayMix      = raw ("delay_mix");
    delayPingpong = raw ("delay_pingpong");
    reduxEnabled  = raw ("redux_enabled");
    reduxBits     = raw ("redux_bits");
    reduxRate     = raw ("redux_rate");
    reduxMix      = raw ("redux_mix");
    arpEnabled    = raw ("arp_enabled");
    arpMode       = raw ("arp_mode");
    arpChord      = raw ("arp_chord");
    arpOctaves    = raw ("arp_octaves");
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
    p.reverbEnabled = v (reverbEnabled) > 0.5f;
    p.reverbMode    = (int) v (reverbMode);
    p.reverbMix     = v (reverbMix);

    p.delayEnabled  = v (delayEnabled) > 0.5f;
    p.delayTime     = v (delayTime);
    p.delayFeedback = v (delayFeedback);
    p.delayMix      = v (delayMix);
    p.delayPingpong = v (delayPingpong) > 0.5f;

    p.reduxEnabled  = v (reduxEnabled) > 0.5f;
    p.reduxBits     = (int) v (reduxBits);
    p.reduxRate     = (int) v (reduxRate);
    p.reduxMix      = v (reduxMix);

    p.arpEnabled    = v (arpEnabled) > 0.5f;
    p.arpMode       = (int) v (arpMode);
    p.arpChord      = (int) v (arpChord);
    p.arpOctaves    = (int) v (arpOctaves);
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

void MetuliferAudioProcessor::valueTreeChildAdded (juce::ValueTree& parent, juce::ValueTree&)
{
    if (parent == modTree)
        rebuildCables();
}

void MetuliferAudioProcessor::valueTreeChildRemoved (juce::ValueTree& parent, juce::ValueTree&, int)
{
    if (parent == modTree)
        rebuildCables();
}

MetuliferAudioProcessor::DestField MetuliferAudioProcessor::fieldForDest (const juce::String& paramId)
{
    const auto dot = paramId.indexOfChar ('_');
    const auto suffix = dot >= 0 ? paramId.substring (dot + 1) : juce::String();

    if (suffix == "a1")      return fA1;
    if (suffix == "d1")      return fD1;
    if (suffix == "s1")      return fS1;
    if (suffix == "r1")      return fR1;
    if (suffix == "a2")      return fA2;
    if (suffix == "d2")      return fD2;
    if (suffix == "s2")      return fS2;
    if (suffix == "r2")      return fR2;
    if (suffix == "mix1")    return fMix1;
    if (suffix == "mix2")        return fMix2;
    if (suffix == "cutoff")      return fCutoff;
    if (suffix == "reverb_mix")  return fReverbMix;
    if (suffix == "delay_time")  return fDelayTime;
    if (suffix == "delay_feedback") return fDelayFeedback;
    if (suffix == "delay_mix")   return fDelayMix;
    if (suffix == "redux_bits")  return fReduxBits;
    if (suffix == "redux_rate")  return fReduxRate;
    if (suffix == "redux_mix")   return fReduxMix;

    return fInvalid;
}

void MetuliferAudioProcessor::setField (Voice::Params& p, DestField field, float value)
{
    switch (field)
    {
        case fA1:            p.attack1   = value; break;
        case fD1:            p.decay1    = value; break;
        case fS1:            p.sustain1  = value; break;
        case fR1:            p.release1  = value; break;
        case fA2:            p.attack2   = value; break;
        case fD2:            p.decay2    = value; break;
        case fS2:            p.sustain2  = value; break;
        case fR2:            p.release2  = value; break;
        case fMix1:          p.mixDb1    = value; break;
        case fMix2:          p.mixDb2    = value; break;
        case fCutoff:        p.cutoff    = value; break;
        case fReverbMix:     p.reverbMix = juce::jlimit (0.0f, 100.0f, value); break;
        case fDelayTime:     p.delayTime = juce::jlimit (20.0f, 1000.0f, value); break;
        case fDelayFeedback: p.delayFeedback = juce::jlimit (0.0f, 90.0f, value); break;
        case fDelayMix:      p.delayMix = juce::jlimit (0.0f, 100.0f, value); break;
        case fReduxBits:     p.reduxBits = juce::jlimit (2, 16, (int) std::round (value)); break;
        case fReduxRate:     p.reduxRate = juce::jlimit (1, 32, (int) std::round (value)); break;
        case fReduxMix:      p.reduxMix = juce::jlimit (0.0f, 100.0f, value); break;
        case fInvalid:       break;
    }
}

void MetuliferAudioProcessor::rebuildCables()
{
    const int target = 1 - activeBuffer.load (std::memory_order_relaxed);
    auto& buffer = cableBuffers[target];
    int count = 0;

    for (int i = 0; i < modTree.getNumChildren() && count < maxCables; ++i)
    {
        const auto child = modTree.getChild (i);

        if (! child.hasType ("cable"))
            continue;

        RuntimeCable cable;
        cable.lfo = juce::jlimit (0, 1, (int) child.getProperty ("src", 0));
        cable.field = fieldForDest (child.getProperty ("dest").toString());

        if (cable.field == fInvalid)
            continue;

        const auto destStr = child.getProperty ("dest").toString();
        cable.param = apvts.getParameter (destStr);
        cable.raw = apvts.getRawParameterValue (destStr);

        cable.seqIndex = -1;
        if (destStr.startsWith ("seq"))
        {
            const auto rest = destStr.substring (3);
            cable.seqIndex = rest.upToFirstOccurrenceOf ("_", false, false).getIntValue() - 1;
        }

        if (cable.param == nullptr || cable.raw == nullptr
             || ! juce::isPositiveAndBelow (cable.seqIndex, numSequences))
            continue;

        buffer[count++] = cable;
    }

    cableCounts[target].store (count, std::memory_order_release);
    activeBuffer.store (target, std::memory_order_release);
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

//==============================================================================
struct FactoryPreset
{
    const char* name;
    float tempo;
    int patterns[6];
    int lengths[6];
    float cutoff[6];
    int wave1[6];
    int wave2[6];
    int oct1[6];
    int note1[6];
    bool delayEnabled[6];
    float delayTime[6];
    float delayFeedback[6];
    float delayMix[6];
    bool delayPingpong[6];
    bool reverbEnabled[6];
    int reverbMode[6];
    float reverbMix[6];
    bool reduxEnabled[6];
    int reduxBits[6];
    int reduxRate[6];
    float reduxMix[6];
    bool arpEnabled[6];
    int arpChord[6];
};

static const FactoryPreset factoryPresets[8] =
{
    // 0: Init Patch
    {
        "01: Init Patch", 120.0f,
        { 0x55555555, 0, 0, 0, 0, 0 },
        { 16, 16, 16, 16, 16, 16 },
        { 20000.0f, 20000.0f, 20000.0f, 20000.0f, 20000.0f, 20000.0f },
        { 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1 },
        { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 },
        { false, false, false, false, false, false }, { 250, 250, 250, 250, 250, 250 },
        { 40, 40, 40, 40, 40, 40 }, { 30, 30, 30, 30, 30, 30 }, { true, true, true, true, true, true },
        { false, false, false, false, false, false }, { 0, 0, 0, 0, 0, 0 }, { 30, 30, 30, 30, 30, 30 },
        { false, false, false, false, false, false }, { 8, 8, 8, 8, 8, 8 }, { 4, 4, 4, 4, 4, 4 }, { 50, 50, 50, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    },
    // 1: Berlin Minimal
    {
        "02: Berlin Minimal", 128.0f,
        { 0b1000100010001000, 0b00100010001000, 0b000100010001, 0, 0, 0 },
        { 16, 14, 12, 16, 16, 16 },
        { 800.0f, 3200.0f, 7500.0f, 20000.0f, 20000.0f, 20000.0f },
        { 0, 1, 2, 1, 1, 1 }, { 0, 1, 2, 1, 1, 1 },
        { -2, 0, 1, 0, 0, 0 }, { 0, 7, 3, 0, 0, 0 },
        { false, true, true, false, false, false }, { 250, 375, 187, 250, 250, 250 },
        { 40, 55, 60, 40, 40, 40 }, { 0, 45, 50, 0, 0, 0 }, { true, true, true, true, true, true },
        { false, true, true, false, false, false }, { 0, 0, 1, 0, 0, 0 }, { 0, 25, 40, 0, 0, 0 },
        { false, false, false, false, false, false }, { 8, 8, 8, 8, 8, 8 }, { 4, 4, 4, 4, 4, 4 }, { 50, 50, 50, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    },
    // 2: Acid 303 Modular
    {
        "03: Acid 303 Modular", 134.0f,
        { 0b1011010110110101, 0b0000100000001000, 0, 0, 0, 0 },
        { 16, 16, 16, 16, 16, 16 },
        { 1400.0f, 900.0f, 20000.0f, 20000.0f, 20000.0f, 20000.0f },
        { 1, 2, 1, 1, 1, 1 }, { 2, 2, 1, 1, 1, 1 },
        { -1, -2, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 },
        { true, false, false, false, false, false }, { 224, 250, 250, 250, 250, 250 },
        { 65, 40, 40, 40, 40, 40 }, { 40, 0, 0, 0, 0, 0 }, { true, true, true, true, true, true },
        { false, false, false, false, false, false }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 },
        { false, false, false, false, false, false }, { 8, 8, 8, 8, 8, 8 }, { 4, 4, 4, 4, 4, 4 }, { 50, 50, 50, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    },
    // 3: Ambient Glass Cloud
    {
        "04: Ambient Glass Cloud", 90.0f,
        { 0b1000000010000000, 0b0000100000000010, 0b0010000000100000, 0, 0, 0 },
        { 16, 16, 16, 16, 16, 16 },
        { 3500.0f, 4200.0f, 2800.0f, 20000.0f, 20000.0f, 20000.0f },
        { 3, 0, 3, 1, 1, 1 }, { 0, 3, 0, 1, 1, 1 },
        { 0, 1, 2, 0, 0, 0 }, { 0, 7, 4, 0, 0, 0 },
        { true, true, true, false, false, false }, { 500, 333, 666, 250, 250, 250 },
        { 70, 75, 80, 40, 40, 40 }, { 55, 60, 50, 0, 0, 0 }, { true, true, true, true, true, true },
        { true, true, true, false, false, false }, { 1, 1, 1, 0, 0, 0 }, { 75, 80, 70, 0, 0, 0 },
        { false, false, false, false, false, false }, { 8, 8, 8, 8, 8, 8 }, { 4, 4, 4, 4, 4, 4 }, { 50, 50, 50, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    },
    // 4: Lo-Fi Polyrhythm
    {
        "05: Lo-Fi Polyrhythm", 112.0f,
        { 0b1001001001001001, 0b100010001000100, 0b1000010000100, 0, 0, 0 },
        { 16, 15, 13, 16, 16, 16 },
        { 2200.0f, 1800.0f, 3000.0f, 20000.0f, 20000.0f, 20000.0f },
        { 2, 1, 2, 1, 1, 1 }, { 1, 2, 1, 1, 1, 1 },
        { -1, 0, 1, 0, 0, 0 }, { 0, 5, 9, 0, 0, 0 },
        { true, true, false, false, false, false }, { 268, 402, 250, 250, 250, 250 },
        { 50, 45, 40, 40, 40, 40 }, { 40, 35, 0, 0, 0, 0 }, { true, true, true, true, true, true },
        { true, false, false, false, false, false }, { 0, 0, 0, 0, 0, 0 }, { 30, 0, 0, 0, 0, 0 },
        { true, true, true, false, false, false }, { 8, 7, 6, 8, 8, 8 }, { 4, 6, 8, 4, 4, 4 }, { 60, 50, 55, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    },
    // 5: Industrial Machine
    {
        "06: Industrial Machine", 140.0f,
        { 0b1100110011001100, 0b0011001100110011, 0b1010101010101010, 0, 0, 0 },
        { 16, 16, 16, 16, 16, 16 },
        { 900.0f, 1500.0f, 2400.0f, 20000.0f, 20000.0f, 20000.0f },
        { 2, 1, 2, 1, 1, 1 }, { 2, 2, 1, 1, 1, 1 },
        { -2, -1, 0, 0, 0, 0 }, { 0, 0, 6, 0, 0, 0 },
        { false, true, true, false, false, false }, { 214, 107, 107, 250, 250, 250 },
        { 40, 70, 75, 40, 40, 40 }, { 0, 45, 50, 0, 0, 0 }, { true, true, true, true, true, true },
        { false, false, false, false, false, false }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 },
        { true, true, true, false, false, false }, { 6, 8, 6, 8, 8, 8 }, { 8, 4, 6, 4, 4, 4 }, { 70, 60, 65, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    },
    // 6: Melodic Arps
    {
        "07: Melodic Arps", 124.0f,
        { 0b1000100010001000, 0b0010001000100010, 0b0001000100010001, 0, 0, 0 },
        { 16, 16, 16, 16, 16, 16 },
        { 5000.0f, 6000.0f, 4500.0f, 20000.0f, 20000.0f, 20000.0f },
        { 1, 1, 3, 1, 1, 1 }, { 3, 3, 1, 1, 1, 1 },
        { 0, 0, 1, 0, 0, 0 }, { 0, 4, 7, 0, 0, 0 },
        { true, true, true, false, false, false }, { 242, 363, 181, 250, 250, 250 },
        { 55, 60, 65, 40, 40, 40 }, { 45, 50, 50, 0, 0, 0 }, { true, true, true, true, true, true },
        { true, true, true, false, false, false }, { 0, 1, 0, 0, 0, 0 }, { 35, 40, 30, 0, 0, 0 },
        { false, false, false, false, false, false }, { 8, 8, 8, 8, 8, 8 }, { 4, 4, 4, 4, 4, 4 }, { 50, 50, 50, 50, 50, 50 },
        { true, true, true, false, false, false }, { 1, 0, 4, 1, 1, 1 }
    },
    // 7: Cyberpunk Drive
    {
        "08: Cyberpunk Drive", 132.0f,
        { 0b1010110010101100, 0b0010001000100010, 0b1100110011001100, 0, 0, 0 },
        { 16, 16, 16, 16, 16, 16 },
        { 2800.0f, 4000.0f, 1600.0f, 20000.0f, 20000.0f, 20000.0f },
        { 1, 2, 1, 1, 1, 1 }, { 2, 1, 2, 1, 1, 1 },
        { -1, 0, -2, 0, 0, 0 }, { 0, 7, 0, 0, 0, 0 },
        { true, true, false, false, false, false }, { 227, 340, 250, 250, 250, 250 },
        { 60, 65, 40, 40, 40, 40 }, { 45, 50, 0, 0, 0, 0 }, { true, true, true, true, true, true },
        { true, true, false, false, false, false }, { 1, 0, 0, 0, 0, 0 }, { 40, 30, 0, 0, 0, 0 },
        { true, false, true, false, false, false }, { 7, 8, 6, 8, 8, 8 }, { 4, 4, 6, 4, 4, 4 }, { 55, 50, 60, 50, 50, 50 },
        { false, false, false, false, false, false }, { 1, 1, 1, 1, 1, 1 }
    }
};

int MetuliferAudioProcessor::getNumPrograms()
{
    return 8;
}

int MetuliferAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void MetuliferAudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, 8))
        return;

    currentProgram = index;
    const auto& pr = factoryPresets[index];

    if (auto* p = apvts.getParameter ("tempo"))
        p->setValueNotifyingHost (p->convertTo0to1 (pr.tempo));

    for (int s = 0; s < numSequences; ++s)
    {
        setStepPattern (s, pr.patterns[s]);

        const auto setP = [this, s] (const char* name, float val)
        {
            if (auto* p = apvts.getParameter (seqParamId (s, name)))
                p->setValueNotifyingHost (p->convertTo0to1 (val));
        };

        setP ("length", (float) pr.lengths[s]);
        setP ("cutoff", pr.cutoff[s]);
        setP ("wave1", (float) pr.wave1[s]);
        setP ("wave2", (float) pr.wave2[s]);
        setP ("octave1", (float) pr.oct1[s]);
        setP ("note1", (float) pr.note1[s]);
        setP ("delay_enabled", pr.delayEnabled[s] ? 1.0f : 0.0f);
        setP ("delay_time", pr.delayTime[s]);
        setP ("delay_feedback", pr.delayFeedback[s]);
        setP ("delay_mix", pr.delayMix[s]);
        setP ("delay_pingpong", pr.delayPingpong[s] ? 1.0f : 0.0f);
        setP ("reverb_enabled", pr.reverbEnabled[s] ? 1.0f : 0.0f);
        setP ("reverb_mode", (float) pr.reverbMode[s]);
        setP ("reverb_mix", pr.reverbMix[s]);
        setP ("redux_enabled", pr.reduxEnabled[s] ? 1.0f : 0.0f);
        setP ("redux_bits", (float) pr.reduxBits[s]);
        setP ("redux_rate", (float) pr.reduxRate[s]);
        setP ("redux_mix", pr.reduxMix[s]);
        setP ("arp_enabled", pr.arpEnabled[s] ? 1.0f : 0.0f);
        setP ("arp_chord", (float) pr.arpChord[s]);
    }
}

const juce::String MetuliferAudioProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, 8) ? factoryPresets[index].name : juce::String();
}

void MetuliferAudioProcessor::changeProgramName (int, const juce::String&)
{
}

bool MetuliferAudioProcessor::isHostSyncEnabled() const
{
    return hostSyncParam != nullptr && hostSyncParam->load (std::memory_order_relaxed) > 0.5f;
}

void MetuliferAudioProcessor::setHostSyncEnabled (bool enabled)
{
    if (auto* p = apvts.getParameter ("host_sync"))
        p->setValueNotifyingHost (enabled ? 1.0f : 0.0f);
}

void MetuliferAudioProcessor::shiftSequence (int seqIndex, int delta)
{
    if (! juce::isPositiveAndBelow (seqIndex, numSequences))
        return;

    const int pat = getStepPattern (seqIndex);
    int len = 32;
    if (auto* raw = apvts.getRawParameterValue (seqParamId (seqIndex, "length")))
        len = juce::jlimit (1, 32, juce::roundToInt (raw->load (std::memory_order_relaxed)));

    int newPat = 0;
    for (int i = 0; i < len; ++i)
    {
        if ((pat & (1 << i)) != 0)
        {
            int target = (i + delta) % len;
            if (target < 0)
                target += len;
            newPat |= (1 << target);
        }
    }

    setStepPattern (seqIndex, newPat);
}

void MetuliferAudioProcessor::randomizeSequence (int seqIndex, float density)
{
    if (! juce::isPositiveAndBelow (seqIndex, numSequences))
        return;

    int len = 32;
    if (auto* raw = apvts.getRawParameterValue (seqParamId (seqIndex, "length")))
        len = juce::jlimit (1, 32, juce::roundToInt (raw->load (std::memory_order_relaxed)));

    const float d = juce::jlimit (0.05f, 0.95f, density);
    int newPat = 0;
    juce::Random rng;

    for (int i = 0; i < len; ++i)
    {
        if (rng.nextFloat() < d)
            newPat |= (1 << i);
    }

    setStepPattern (seqIndex, newPat);
}

void MetuliferAudioProcessor::euclideanSequence (int seqIndex, int hits, int length)
{
    if (! juce::isPositiveAndBelow (seqIndex, numSequences))
        return;

    const int n = juce::jlimit (1, 32, length);
    const int k = juce::jlimit (0, n, hits);

    if (k == 0)
    {
        setStepPattern (seqIndex, 0);
        return;
    }

    int newPat = 0;
    for (int i = 0; i < n; ++i)
    {
        if ((i * k) % n < k)
            newPat |= (1 << i);
    }

    setStepPattern (seqIndex, newPat);
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
void MetuliferAudioProcessor::prepareToPlay (double sampleRateIn, int samplesPerBlock)
{
    sampleRate = sampleRateIn;

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

void MetuliferAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // 1. Process incoming MIDI (Transposition relative to middle C)
    for (const auto meta : midiMessages)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
            midiTranspose.store (msg.getNoteNumber() - 60, std::memory_order_relaxed);
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            midiTranspose.store (0, std::memory_order_relaxed);
    }
    midiMessages.clear();

    // 2. Host Transport & Tempo Sync
    const bool syncEnabled = hostSyncParam != nullptr
                             && hostSyncParam->load (std::memory_order_relaxed) > 0.5f;

    if (syncEnabled)
    {
        if (auto* playHead = getPlayHead())
        {
            if (auto posOpt = playHead->getPosition())
            {
                const bool isHostPlaying = posOpt->getIsPlaying();
                const double bpm = posOpt->getBpm().orFallback (0.0);
                const double ppq = posOpt->getPpqPosition().orFallback (0.0);
                engine.syncToHost (ppq, bpm, isHostPlaying);
            }
        }
    }

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    updateLfos (numSamples);

    const int transpose = midiTranspose.load (std::memory_order_relaxed);

    Voice::Params params[numSequences];
    for (int s = 0; s < numSequences; ++s)
    {
        params[s] = voiceSources[s].read();
        params[s].transpose = transpose;
    }

    {
        const int active = activeBuffer.load (std::memory_order_acquire);
        const int cableCount = cableCounts[active].load (std::memory_order_acquire);

        for (int c = 0; c < cableCount; ++c)
        {
            const auto& cable = cableBuffers[active][c];

            const bool enabled = lfoEnabled[cable.lfo] != nullptr
                                 && lfoEnabled[cable.lfo]->load (std::memory_order_relaxed) > 0.5f;
            if (! enabled)
                continue;

            const float lfoVal = lfoValue[cable.lfo].load (std::memory_order_relaxed);
            float norm = cable.param->convertTo0to1 (cable.raw->load (std::memory_order_relaxed));
            norm = juce::jlimit (0.0f, 1.0f, norm + lfoVal * 0.5f);
            setField (params[cable.seqIndex], cable.field, cable.param->convertFrom0to1 (norm));
        }
    }

    for (int s = 0; s < numSequences; ++s)
        voices[s].setParams (params[s]);

    if (engine.takePendingNoteOff())
    {
        for (int s = 0; s < numSequences; ++s)
        {
            voices[s].noteOff();
            voices[s].resetArp();
            if (activeMidiNotes[s] >= 0)
            {
                midiMessages.addEvent (juce::MidiMessage::noteOff (s + 1, activeMidiNotes[s], 0.0f), 0);
                activeMidiNotes[s] = -1;
            }
        }
    }

    const auto& events = engine.process (numSamples);

    int pos = 0;

    for (const auto& ev : events)
    {
        if (ev.sampleOffset > pos)
            renderVoices (buffer, pos, ev.sampleOffset - pos);

        const int seq = ev.sequence;
        auto& voice = voices[seq];
        const int channel = seq + 1;
        const int midiNote = juce::jlimit (0, 127, 48 + params[seq].octave1 * 12 + params[seq].note1 + transpose);

        if (ev.gateOn)
        {
            voice.noteOn();
            if (activeMidiNotes[seq] >= 0)
                midiMessages.addEvent (juce::MidiMessage::noteOff (channel, activeMidiNotes[seq], 0.0f), ev.sampleOffset);
            activeMidiNotes[seq] = midiNote;
            midiMessages.addEvent (juce::MidiMessage::noteOn (channel, midiNote, 0.85f), ev.sampleOffset);
        }
        else
        {
            voice.noteOff();
            if (activeMidiNotes[seq] >= 0)
            {
                midiMessages.addEvent (juce::MidiMessage::noteOff (channel, activeMidiNotes[seq], 0.0f), ev.sampleOffset);
                activeMidiNotes[seq] = -1;
            }
        }

        pos = ev.sampleOffset;
    }

    if (pos < numSamples)
        renderVoices (buffer, pos, numSamples - pos);

    // Headroom scaling for summing 6 voices + smooth soft saturation
    // to prevent DAC / audio interface digital hard clipping.
    buffer.applyGain (0.5f);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = std::tanh (data[i]);
    }

    // Accurate dB meter calculation (Peak and RMS in dBFS per channel)
    {
        float maxL = 0.0f;
        float maxR = 0.0f;
        float sumSqL = 0.0f;
        float sumSqR = 0.0f;

        if (numChannels > 0)
        {
            const auto* l = buffer.getReadPointer (0);
            for (int i = 0; i < numSamples; ++i)
            {
                const float s = std::abs (l[i]);
                if (s > maxL) maxL = s;
                sumSqL += s * s;
            }
        }
        if (numChannels > 1)
        {
            const auto* r = buffer.getReadPointer (1);
            for (int i = 0; i < numSamples; ++i)
            {
                const float s = std::abs (r[i]);
                if (s > maxR) maxR = s;
                sumSqR += s * s;
            }
        }
        else
        {
            maxR = maxL;
            sumSqR = sumSqL;
        }

        const float rmsLVal = numSamples > 0 ? std::sqrt (sumSqL / (float) numSamples) : 0.0f;
        const float rmsRVal = numSamples > 0 ? std::sqrt (sumSqR / (float) numSamples) : 0.0f;

        const float dbPeakL = maxL > 1e-5f ? 20.0f * std::log10 (maxL) : -80.0f;
        const float dbPeakR = maxR > 1e-5f ? 20.0f * std::log10 (maxR) : -80.0f;
        const float dbRmsL  = rmsLVal > 1e-5f ? 20.0f * std::log10 (rmsLVal) : -80.0f;
        const float dbRmsR  = rmsRVal > 1e-5f ? 20.0f * std::log10 (rmsRVal) : -80.0f;

        // Hold peak between UI timer frames so transient notes are never missed or delayed
        const float prevPeakL = peakDbL.load (std::memory_order_relaxed);
        if (dbPeakL > prevPeakL)
            peakDbL.store (dbPeakL, std::memory_order_relaxed);
        else
            peakDbL.store (juce::jmax (dbPeakL, prevPeakL - 0.6f), std::memory_order_relaxed);

        const float prevPeakR = peakDbR.load (std::memory_order_relaxed);
        if (dbPeakR > prevPeakR)
            peakDbR.store (dbPeakR, std::memory_order_relaxed);
        else
            peakDbR.store (juce::jmax (dbPeakR, prevPeakR - 0.6f), std::memory_order_relaxed);

        rmsDbL.store (dbRmsL, std::memory_order_relaxed);
        rmsDbR.store (dbRmsR, std::memory_order_relaxed);

        const float combinedRms = (rmsLVal + rmsRVal) * 0.5f;
        rms.store (combinedRms, std::memory_order_relaxed);
    }

    // Oscilloscope ring buffer (lock-free, realtime safe).
    if (numChannels > 0)
    {
        const auto* l = buffer.getReadPointer (0);
        const auto* r = numChannels > 1 ? buffer.getReadPointer (1) : l;
        int writePos = scopeWriteIndex.load (std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i)
        {
            scopeBuffer[writePos] = (l[i] + r[i]) * 0.5f;
            writePos = (writePos + 1) % scopeBufferSize;
        }
        scopeWriteIndex.store (writePos, std::memory_order_release);
    }
}

void MetuliferAudioProcessor::getScopeBuffer (float* dest, int numSamplesToRead) const
{
    const int count = juce::jmin (numSamplesToRead, scopeBufferSize);
    const int writePos = scopeWriteIndex.load (std::memory_order_acquire);
    int readPos = (writePos - count + scopeBufferSize) % scopeBufferSize;
    for (int i = 0; i < count; ++i)
    {
        dest[i] = scopeBuffer[readPos];
        readPos = (readPos + 1) % scopeBufferSize;
    }
}

void MetuliferAudioProcessor::renderVoices (juce::AudioBuffer<float>& buffer,
                                            int startSample, int numSamples)
{
    for (auto& voice : voices)
        voice.render (buffer, startSample, numSamples);
}

void MetuliferAudioProcessor::updateLfos (int numSamples)
{
    const double dt = (double) numSamples / juce::jmax (1.0, sampleRate);

    for (int i = 0; i < 2; ++i)
    {
        const bool enabled = lfoEnabled[i] != nullptr
                             && lfoEnabled[i]->load (std::memory_order_relaxed) > 0.5f;

        if (! enabled)
        {
            lfoValue[i].store (0.0f, std::memory_order_relaxed);
            continue;
        }

        const float rate = juce::jlimit (0.1f, 20.0f, lfoRate[i]->load (std::memory_order_relaxed));
        lfoPhase[i] += rate * dt;
        lfoPhase[i] -= std::floor (lfoPhase[i]);

        const float p = (float) lfoPhase[i];
        float w = 0.0f;

        switch ((int) lfoWave[i]->load (std::memory_order_relaxed))
        {
            case 0:  w = std::sin (p * juce::MathConstants<float>::twoPi); break;
            case 1:  w = 2.0f * p - 1.0f; break;
            case 2:  w = p < 0.5f ? 1.0f : -1.0f; break;
            case 3:  w = 1.0f - 4.0f * std::abs (p - 0.5f); break;
            default: break;
        }

        const float depth = juce::jlimit (0.0f, 1.0f, lfoDepth[i]->load (std::memory_order_relaxed) / 100.0f);
        const float offset = juce::jlimit (-1.0f, 1.0f, lfoOffset[i]->load (std::memory_order_relaxed) / 100.0f);

        lfoValue[i].store (juce::jlimit (-1.0f, 1.0f, w * depth + offset), std::memory_order_relaxed);
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
    root.appendChild (modTree.createCopy(), nullptr);

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

    auto modState = root.getChildWithName (modTree.getType());
    if (modState.isValid())
    {
        modTree.copyPropertiesAndChildrenFrom (modState, nullptr);
        rebuildCables();
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    static bool hasRunTests = false;
    if (! hasRunTests)
    {
        hasRunTests = true;
        bool shouldRunTests = false;
        if (const char* env = std::getenv ("METULIFER_RUN_TESTS"))
            if (env[0] != '0') shouldRunTests = true;

        if (juce::JUCEApplicationBase::isStandaloneApp())
        {
            if (juce::JUCEApplicationBase::getCommandLineParameters().contains ("--run-tests"))
                shouldRunTests = true;
        }

        if (shouldRunTests)
        {
            struct ConsoleTestRunner : public juce::UnitTestRunner
            {
                void logMessage (const juce::String& msg) override
                {
                    std::cout << msg.toRawUTF8() << std::endl;
                }
            };

            ConsoleTestRunner runner;
            runner.setAssertOnFailure (false);
            runner.setPassesAreLogged (true);
            runner.runAllTests (true);

            int numFailures = 0;
            for (int i = 0; i < runner.getNumResults(); ++i)
                numFailures += runner.getResult (i)->failures;

            std::cout << "\n========================================\n";
            std::cout << "TEST SUMMARY: " << (numFailures == 0 ? "ALL PASSED" : "FAILURES DETECTED: " + std::to_string (numFailures)) << "\n";
            std::cout << "========================================\n" << std::endl;

            if (auto* app = juce::JUCEApplicationBase::getInstance())
                app->systemRequestedQuit();
            else
                std::exit (numFailures > 0 ? 1 : 0);
        }
    }
    return new MetuliferAudioProcessor();
}
