#pragma once

#include <JuceHeader.h>

class Lever : public juce::Component
{
public:
    Lever();

    /** Rebinds to an integer parameter (-2..+2). */
    void setParam (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId);

    int getValue() const noexcept { return value; }

    /** Fired on user interaction (for the display screen). */
    std::function<void (int)> onValueChanged;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    static constexpr int numPositions = 5;

    int positionFromY (float y) const;
    void applyFromMouse (const juce::MouseEvent& e);
    void setValue (int newValue);

    std::unique_ptr<juce::ParameterAttachment> attachment;
    int value = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Lever)
};
