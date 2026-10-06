#pragma once

#include <JuceHeader.h>

class DisplayScreen : public juce::Component
{
public:
    DisplayScreen();

    void paint (juce::Graphics& g) override;

    void setSequenceNumber (int index);
    void setParamText (const juce::String& text);
    void setRmsLevel (float rms);

private:
    int sequenceNumber = 1;
    juce::String paramText { "—" };
    float rms = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplayScreen)
};
