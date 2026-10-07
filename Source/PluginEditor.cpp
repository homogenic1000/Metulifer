/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/Skin.h"

//==============================================================================
MetuliferAudioProcessorEditor::MetuliferAudioProcessorEditor (MetuliferAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      sequencerPanel (p.apvts)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (sequencerPanel);
    addAndMakeVisible (displayScreen);
    addAndMakeVisible (clockPanel);
    addAndMakeVisible (synthPanel);
    addAndMakeVisible (vcfPanel);

    sequencerPanel.onSelectionChanged = [this] (int index)
    {
        sequenceSelected (index);
    };

    vcfPanel.onFilterToggled = [this] (bool useLpf)
    {
        setFilterTypeParam (useLpf);
    };

    // Knobs bound once and for all (one row = one sequence).
    auto track = [this] (juce::Slider& knob, const juce::String& paramName, const juce::String& display)
    {
        knob.setName (display);
        sliderParamIds[&knob] = paramName;
        knob.addListener (this);
    };

    for (int i = 0; i < MetuliferAudioProcessor::numSequences; ++i)
        if (auto* row = sequencerPanel.getRow (i))
        {
            track (row->getLengthKnob(), MetuliferAudioProcessor::seqParamId (i, "length"), "length");
            track (row->getVolumeKnob(), MetuliferAudioProcessor::seqParamId (i, "volume"), "volume");
        }

    track (clockPanel.tempoKnob, "tempo", "tempo");

    // Knobs re-bound on every sequence selection (ids/display set in bindSequence).
    for (auto* knob : { &synthPanel.vco1Octave, &synthPanel.vco1Wave,
                        &synthPanel.vco2Octave, &synthPanel.vco2Wave,
                        &synthPanel.adsr1Attack, &synthPanel.adsr1Decay,
                        &synthPanel.adsr1Sustain, &synthPanel.adsr1Release,
                        &synthPanel.adsr2Attack, &synthPanel.adsr2Decay,
                        &synthPanel.adsr2Sustain, &synthPanel.adsr2Release,
                        &synthPanel.mixVco1, &synthPanel.mixVco2,
                        &vcfPanel.cutoffKnob })
        knob->addListener (this);

    bindSequence (0);
    displayScreen.setSequenceNumber (1);

    sequencerPanel.setStepsTree (audioProcessor.getStepsTree());

    startTimerHz (30);
    setSize (1280, 720);
}

MetuliferAudioProcessorEditor::~MetuliferAudioProcessorEditor()
{
    setLookAndFeel (nullptr);

    if (filterParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (filterParamId, this);
}

//==============================================================================
void MetuliferAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (auto img = Skin::find ("bg_editor"); img.isValid())
        g.drawImage (img, getLocalBounds().toFloat(),
                     juce::RectanglePlacement::stretchToFit, false);
    else
        g.fillAll (juce::Colour (0xffe9e9e9));

    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("METULIFER", 32, 8, 200, 20, juce::Justification::centredLeft);
}

void MetuliferAudioProcessorEditor::resized()
{
    sequencerPanel.setBounds (32, 32, 910, 417);
    displayScreen.setBounds (986, 32, 247, 132);
    clockPanel.setBounds (961, 164, 297, 284);
    synthPanel.setBounds (32, 458, 786, 237);
    vcfPanel.setBounds (684, 471, 105, 224);
}

//==============================================================================
void MetuliferAudioProcessorEditor::sequenceSelected (int index)
{
    displayScreen.setSequenceNumber (index + 1);
    bindSequence (index);
    clearParamDisplay();
}

void MetuliferAudioProcessorEditor::bindSequence (int index)
{
    currentSeq = index;
    updatingBindings = true;

    if (filterParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (filterParamId, this);

    synthAttachments.clear();
    vcfAttachments.clear();

    auto addKnob = [this] (std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>& target,
                           juce::Slider& knob, const juce::String& paramName, const juce::String& display)
    {
        knob.setName (display);
        sliderParamIds[&knob] = paramName;
        target.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, paramName, knob));
    };

    const auto id = [index] (const juce::String& n)
    {
        return MetuliferAudioProcessor::seqParamId (index, n);
    };

    addKnob (synthAttachments, synthPanel.vco1Octave,   id ("octave1"), "octave VCO1");
    addKnob (synthAttachments, synthPanel.vco1Wave,     id ("wave1"),   "wave VCO1");
    addKnob (synthAttachments, synthPanel.vco2Octave,   id ("octave2"), "octave VCO2");
    addKnob (synthAttachments, synthPanel.vco2Wave,     id ("wave2"),   "wave VCO2");

    addKnob (synthAttachments, synthPanel.adsr1Attack,  id ("a1"), "attack VCO1");
    addKnob (synthAttachments, synthPanel.adsr1Decay,   id ("d1"), "decay VCO1");
    addKnob (synthAttachments, synthPanel.adsr1Sustain, id ("s1"), "sustain VCO1");
    addKnob (synthAttachments, synthPanel.adsr1Release, id ("r1"), "release VCO1");

    addKnob (synthAttachments, synthPanel.adsr2Attack,  id ("a2"), "attack VCO2");
    addKnob (synthAttachments, synthPanel.adsr2Decay,   id ("d2"), "decay VCO2");
    addKnob (synthAttachments, synthPanel.adsr2Sustain, id ("s2"), "sustain VCO2");
    addKnob (synthAttachments, synthPanel.adsr2Release, id ("r2"), "release VCO2");

    addKnob (synthAttachments, synthPanel.mixVco1,      id ("mix1"),  "mix VCO1");
    addKnob (synthAttachments, synthPanel.mixVco2,      id ("mix2"),  "mix VCO2");

    addKnob (vcfAttachments,   vcfPanel.cutoffKnob,     id ("cutoff"), "cutoff");

    filterParamId = id ("filter");
    audioProcessor.apvts.addParameterListener (filterParamId, this);
    updateFilterButtons();

    updatingBindings = false;
}

//==============================================================================
void MetuliferAudioProcessorEditor::sliderDragStarted (juce::Slider* slider)
{
    if (! updatingBindings && slider != nullptr)
        showParamFor (*slider);
}

void MetuliferAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (! updatingBindings && slider != nullptr)
        showParamFor (*slider);
}

void MetuliferAudioProcessorEditor::showParamFor (juce::Slider& slider)
{
    auto it = sliderParamIds.find (&slider);
    if (it == sliderParamIds.end())
        return;

    auto* param = audioProcessor.apvts.getParameter (it->second);
    if (param == nullptr)
        return;

    const float normalised = param->getNormalisableRange().convertTo0to1 ((float) slider.getValue());

    juce::String text = slider.getName();
    text << " " << param->getText (normalised, 8);

    const auto label = param->getLabel();
    if (label.isNotEmpty())
        text << " " << label;

    showText (text);
}

void MetuliferAudioProcessorEditor::showText (const juce::String& text)
{
    displayScreen.setParamText (text);
    paramShown = true;
    paramShownUntilMs = juce::Time::getMillisecondCounter() + 2000;
}

void MetuliferAudioProcessorEditor::clearParamDisplay()
{
    paramShown = false;
    displayScreen.setParamText ({});
}

//==============================================================================
void MetuliferAudioProcessorEditor::setFilterTypeParam (bool useLpf)
{
    auto* param = audioProcessor.apvts.getParameter (filterParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (useLpf ? 0.0f : 1.0f);
    param->endChangeGesture();
}

void MetuliferAudioProcessorEditor::updateFilterButtons()
{
    auto* param = audioProcessor.apvts.getParameter (filterParamId);
    if (param == nullptr)
        return;

    vcfPanel.setFilterType (param->getValue() < 0.5f);
}

void MetuliferAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float)
{
    if (parameterID != filterParamId)
        return;

    updateFilterButtons();

    auto* param = audioProcessor.apvts.getParameter (parameterID);
    if (param != nullptr)
        showText ("filter " + juce::String (param->getValue() < 0.5f ? "LPF" : "HPF"));
}

//==============================================================================
void MetuliferAudioProcessorEditor::timerCallback()
{
    displayScreen.setRmsLevel (audioProcessor.getCurrentRms());

    if (paramShown && juce::Time::getMillisecondCounter() >= paramShownUntilMs)
        clearParamDisplay();
}
