#pragma once

#include <JuceHeader.h>

class LFOPanel : public juce::Component
{
public:
    explicit LFOPanel (const juce::String& name);

    void paint (juce::Graphics& g) override;
    void resized() override;

    /** Jack de sortie, coordonnées relatives au panneau. */
    juce::Rectangle<float> getJackBounds() const;

    void setWaveIndex (int index);
    void setEnabled (bool on);

    std::function<void (int newWaveIndex)> onWaveClicked;
    std::function<void (bool on)> onPowerClicked;

    juce::Slider rateKnob, offsetKnob, depthKnob;
    juce::TextButton waveButton, onOffButton;

private:
    juce::String panelName;
    juce::Label titleLabel;
    juce::Label rateLabel { "", "Rate" }, offsetLabel { "", "Offset" }, depthLabel { "", "Depth" };

    int waveIndex = 0;
    bool on = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOPanel)
};
