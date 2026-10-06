/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MetuliferAudioProcessorEditor::MetuliferAudioProcessorEditor (MetuliferAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (sequencerPanel);
    addAndMakeVisible (displayScreen);
    addAndMakeVisible (clockPanel);
    addAndMakeVisible (synthPanel);
    addAndMakeVisible (vcfPanel);

    sequencerPanel.onSelectionChanged = [this] (int index)
    {
        sequenceSelected (index);
    };

    displayScreen.setSequenceNumber (1);

    setSize (1280, 720);
}

MetuliferAudioProcessorEditor::~MetuliferAudioProcessorEditor()
{
}

//==============================================================================
void MetuliferAudioProcessorEditor::paint (juce::Graphics& g)
{
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

void MetuliferAudioProcessorEditor::sequenceSelected (int index)
{
    displayScreen.setSequenceNumber (index + 1);
    displayScreen.setParamText ("—");
}
