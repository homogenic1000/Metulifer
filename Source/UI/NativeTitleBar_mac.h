#pragma once

#include <JuceHeader.h>

namespace NativeTitleBar
{
    /**
     * Configures the standalone window to use authentic macOS native traffic light buttons
     * (close, minimise, zoom) with Dark Aqua appearance instead of custom JUCE-drawn buttons.
     */
    void setupNativeTitleBar (juce::DocumentWindow* window);

    /**
     * Opens the Standalone Audio & MIDI Settings menu, optionally anchored to targetComponent.
     */
    void openSettingsMenu (juce::DocumentWindow* window, juce::Component* targetComponent = nullptr);
}
