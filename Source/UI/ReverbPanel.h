#pragma once

#include <JuceHeader.h>

class ReverbPanel : public juce::Component
{
public:
    ReverbPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Tab selection (0: REV, 1: DLY, 2: RDX)
    void setSelectedTab (int tab);
    int getSelectedTab() const noexcept { return currentTab; }

    // Reverb controls
    void setReverbEnabled (bool on);
    void setReverbMode (int mode);
    juce::TextButton revOnOffButton { "Off" };
    juce::TextButton revModeButton { "Room" };
    juce::Slider revMixKnob;

    // Delay controls
    void setDelayEnabled (bool on);
    void setDelayPingpong (bool pp);
    juce::TextButton dlyOnOffButton { "Off" };
    juce::TextButton dlyModeButton { "PingPong" };
    juce::Slider dlyTimeKnob, dlyFeedbackKnob, dlyMixKnob;

    // Redux controls
    void setReduxEnabled (bool on);
    juce::TextButton rdxOnOffButton { "Off" };
    juce::Slider rdxBitsKnob, rdxRateKnob, rdxMixKnob;

    // Callbacks
    std::function<void (bool on)> onReverbPowerToggled;
    std::function<void (int mode)> onReverbModeToggled;
    std::function<void (bool on)> onDelayPowerToggled;
    std::function<void (bool pp)> onDelayModeToggled;
    std::function<void (bool on)> onReduxPowerToggled;

private:
    void updateTabVisibility();

    juce::Label titleLabel { "", "FX" };

    // Tabs
    juce::TextButton tabRev { "REV" }, tabDly { "DLY" }, tabRdx { "RDX" };
    int currentTab = 0; // 0: REV, 1: DLY, 2: RDX

    // Labels
    juce::Label revMixLabel { "", "Mix" };
    juce::Label dlyTimeLabel { "", "Time" }, dlyFeedbackLabel { "", "Feedbk" }, dlyMixLabel { "", "Mix" };
    juce::Label rdxBitsLabel { "", "Bits" }, rdxRateLabel { "", "Rate" }, rdxMixLabel { "", "Mix" };

    bool isRevOn = false, isDlyOn = false, isRdxOn = false;
    int revModeIndex = 0;
    bool isDlyPingpong = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbPanel)
};
