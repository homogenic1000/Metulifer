#pragma once

#include <JuceHeader.h>

class VCFPanel : public juce::Component
{
public:
    VCFPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setFilterType (bool useLpf);
    std::function<void (bool useLpf)> onFilterToggled;

    juce::Slider cutoffKnob;

private:
    void updateFilterType();

    juce::TextButton lpfButton { "LPF" }, hpfButton { "HPF" };
    juce::Label titleLabel { "", "VCF" };
    juce::Label cutoffLabel { "", "Cutoff" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCFPanel)
};
