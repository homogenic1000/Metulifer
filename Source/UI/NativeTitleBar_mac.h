#pragma once

#include <JuceHeader.h>

namespace NativeTitleBar
{
    /**
     * Configures the standalone window to use authentic macOS native traffic light buttons
     * (close, minimise) with Dark Aqua appearance instead of custom JUCE-drawn buttons,
     * locked strictly to a fixed pixel-perfect size (1280x720) for rendered assets.
     */
    void setupNativeTitleBar (juce::DocumentWindow* window, juce::Component* editor = nullptr);

    /**
     * Opens the Standalone Audio & MIDI Settings menu, optionally anchored to targetComponent.
     */
    void openSettingsMenu (juce::DocumentWindow* window, juce::Component* targetComponent = nullptr);
}
