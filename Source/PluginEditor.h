/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SequencerPanel.h"
#include "UI/DisplayScreen.h"
#include "UI/ClockPanel.h"
#include "UI/SynthPanel.h"
#include "UI/VCFPanel.h"

//==============================================================================
/**
*/
class MetuliferAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MetuliferAudioProcessorEditor (MetuliferAudioProcessor&);
    ~MetuliferAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void sequenceSelected (int index);

    MetuliferAudioProcessor& audioProcessor;

    SequencerPanel sequencerPanel;
    DisplayScreen displayScreen;
    ClockPanel clockPanel;
    SynthPanel synthPanel;
    VCFPanel vcfPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetuliferAudioProcessorEditor)
};
