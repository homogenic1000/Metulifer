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
    juce::Rectangle<float> getOledBounds() const;

    void setWaveIndex (int index);
    void setEnabled (bool on);
    bool isEnabled() const noexcept { return on; }

    void setDestinationSummary (const juce::String& summary);
    void setRateHz (float hz);
    void setLfoActivity (float lfoVal);

    std::function<void (int newWaveIndex)> onWaveClicked;
    std::function<void (bool on)> onPowerClicked;

    juce::Slider rateKnob, offsetKnob, depthKnob;
    juce::TextButton waveButton, onOffButton;

private:
    juce::String panelName;
    juce::Label titleLabel;
    juce::Label rateLabel { "", "Rate" }, offsetLabel { "", "Offset" }, depthLabel { "", "Depth" };

    juce::String destinationText { "NO TARGET" };
    juce::String waveNameText { "SIN" };
    float currentRateHz = 1.0f;
    float currentLfoValue = 0.0f;

    int waveIndex = 0;
    bool on = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOPanel)
};
