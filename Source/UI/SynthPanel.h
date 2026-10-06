#pragma once

#include <JuceHeader.h>

class SynthPanel : public juce::Component
{
public:
    SynthPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    juce::Slider vco1Octave, vco1Wave, vco2Octave, vco2Wave;
    juce::Slider adsr1Attack, adsr1Decay, adsr1Sustain, adsr1Release;
    juce::Slider adsr2Attack, adsr2Decay, adsr2Sustain, adsr2Release;
    juce::Slider mixVco1, mixVco2;

private:
    void setupKnob (juce::Slider& knob, const juce::String& name,
                    double min, double max, double step, double def);
    juce::Label* makeLabel (juce::Component& parent, const juce::String& text, float fontHeight);

    std::vector<std::unique_ptr<juce::Label>> labels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthPanel)
};
