#pragma once

#include <JuceHeader.h>

class ReverbPanel : public juce::Component
{
public:
    ReverbPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setEnabledState (bool on);
    void setModeIndex (int mode);

    std::function<void (bool on)> onPowerToggled;
    std::function<void (int mode)> onModeToggled;

    juce::Slider mixKnob;

private:
    void updateButtons();

    juce::TextButton onOffButton { "Off" };
    juce::TextButton modeButton { "Room" };
    juce::Label titleLabel { "", "reverb" };
    juce::Label mixLabel { "", "Mix" };

    bool isEnabled = false;
    int modeIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbPanel)
};
