#pragma once

#include <JuceHeader.h>

class ClockPanel : public juce::Component
{
public:
    ClockPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    juce::Slider tempoKnob;

private:
    juce::Label titleLabel { "", "CLOCK" };
    juce::TextButton playStopButton { "Play" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClockPanel)
};
