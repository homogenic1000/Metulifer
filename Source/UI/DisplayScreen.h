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
    void updateMeterLevels (float peakDbL, float peakDbR, float rmsDbL, float rmsDbR);
    void pushScopeSamples (const float* samples, int numSamples);

private:
    int sequenceNumber = 1;
    juce::String paramText;
    float rms = 0.0f;

    // Accurate dB meter state & ballistics
    float meterPeakL = -80.0f;
    float meterPeakR = -80.0f;
    float peakHoldL = -80.0f;
    float peakHoldR = -80.0f;
    int peakHoldTimerL = 0;
    int peakHoldTimerR = 0;
    float lastDbReadout = -80.0f;

    static constexpr int numScopePoints = 128;
    float scopePoints[numScopePoints] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplayScreen)
};
