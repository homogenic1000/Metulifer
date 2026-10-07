#pragma once

#include <JuceHeader.h>

namespace Skin
{
    /** "VCO1 Octave" -> "vco1_octave". */
    juce::String normalise (const juce::String& name);

    /** Image asset from Resources/ (BinaryData), normalised name; invalid if absent. */
    juce::Image find (const juce::String& assetName);
    bool has (const juce::String& assetName);

    /** Draws assetName scaled to bounds, else a rounded rect fallback. */
    void drawPanel (juce::Graphics& g, const juce::String& assetName,
                    juce::Rectangle<float> bounds, juce::Colour fallbackColour,
                    float cornerRadius);
}
