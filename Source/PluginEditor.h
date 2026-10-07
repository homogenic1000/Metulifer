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
class MetuliferAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       private juce::Slider::Listener,
                                       private juce::Timer,
                                       private juce::AudioProcessorValueTreeState::Listener
{
public:
    MetuliferAudioProcessorEditor (MetuliferAudioProcessor&);
    ~MetuliferAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void sequenceSelected (int index);
    void bindSequence (int index);
    void showParamFor (juce::Slider& slider);
    void showText (const juce::String& text);
    void clearParamDisplay();
    void setFilterTypeParam (bool useLpf);
    void updateFilterButtons();

    void sliderValueChanged (juce::Slider* slider) override;
    void sliderDragStarted (juce::Slider* slider) override;
    void timerCallback() override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    MetuliferAudioProcessor& audioProcessor;

    SequencerPanel sequencerPanel;
    DisplayScreen displayScreen;
    ClockPanel clockPanel;
    SynthPanel synthPanel;
    VCFPanel vcfPanel;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> synthAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> vcfAttachments;
    std::map<juce::Slider*, juce::String> sliderParamIds;

    juce::String filterParamId;
    int currentSeq = 0;
    bool updatingBindings = false;

    bool paramShown = false;
    juce::uint32 paramShownUntilMs = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetuliferAudioProcessorEditor)
};
