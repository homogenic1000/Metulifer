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
#include "UI/LFOPanel.h"
#include "UI/ReverbPanel.h"
#include "UI/CableLayer.h"
#include "UI/ModMatrixPanel.h"
#include "UI/MetuliferLookAndFeel.h"
#include <melatonin_inspector/melatonin_inspector.h>

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
    void parentHierarchyChanged() override;

private:
    void sequenceSelected (int index);
    void bindSequence (int index);
    void showParamFor (juce::Slider& slider);
    void showText (const juce::String& text);
    void clearParamDisplay();
    void setFilterTypeParam (bool useLpf);
    void updateFilterButtons();
    void setReverbPowerParam (bool on);
    void setReverbModeParam (int mode);
    void setDelayPowerParam (bool on);
    void setDelayModeParam (bool pp);
    void setReduxPowerParam (bool on);
    void updateFxButtons();
    void setArpPowerParam (bool on);
    void setArpModeParam (int mode);
    void setArpChordParam (int chord);
    void updateArpButtons();
    void setLfoWave (int lfoIndex, int waveIndex);
    void setLfoPower (int lfoIndex, bool on);
    void updateLfoButtons();
    void updateLfoDestinations();
    void refreshCableTargets();
    void setMatrixViewOpen (bool open);

    void sliderValueChanged (juce::Slider* slider) override;
    void sliderDragStarted (juce::Slider* slider) override;
    void timerCallback() override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    bool keyPressed (const juce::KeyPress& key) override;

    MetuliferAudioProcessor& audioProcessor;

    MetuliferLookAndFeel lookAndFeel;

    SequencerPanel sequencerPanel;
    DisplayScreen displayScreen;
    ClockPanel clockPanel;
    SynthPanel synthPanel;
    VCFPanel vcfPanel;
    LFOPanel lfoPanel1 { "LFO1" }, lfoPanel2 { "LFO2" };
    ReverbPanel reverbPanel;
    CableLayer cableLayer;
    ModMatrixPanel modMatrixPanel;
    juce::TextButton matrixButton { "MATRIX" };
    juce::TextButton settingsButton { "AUDIO" };
    juce::TextButton inspectButton { "Inspect" };
    melatonin::Inspector inspector { *this, false };
    bool isMatrixViewOpen = false;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> synthAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> vcfAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> fxAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> lfoAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> arpAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hostSyncAttachment;
    std::map<juce::Slider*, juce::String> sliderParamIds;

    struct TargetKnob { juce::Slider* knob = nullptr; juce::String suffix; };
    std::vector<TargetKnob> targetKnobs;

    juce::String filterParamId;
    juce::String reverbEnabledParamId, reverbModeParamId;
    juce::String delayEnabledParamId, delayPingpongParamId;
    juce::String reduxEnabledParamId;
    juce::String arpEnabledParamId, arpModeParamId, arpChordParamId;
    int currentSeq = 0;
    bool updatingBindings = false;
    int lastPlayhead[MetuliferAudioProcessor::numSequences] { -1, -1, -1, -1, -1, -1 };

    bool paramShown = false;
    juce::uint32 paramShownUntilMs = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetuliferAudioProcessorEditor)
};
