#pragma once

#include <JuceHeader.h>

class MetuliferLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MetuliferLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

private:
    static juce::Image findButtonImage (const juce::Button& button);

    void drawHardwareKnob (juce::Graphics& g, float x, float y, float width, float height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider);

    void drawStepButtonBackground (juce::Graphics& g, juce::Button& button,
                                   bool isHighlighted, bool isDown);

    void drawStepButtonText (juce::Graphics& g, juce::TextButton& button);

    void drawStandardButtonBackground (juce::Graphics& g, juce::Button& button,
                                       const juce::Colour& backgroundColour,
                                       bool isHighlighted, bool isDown);
};
