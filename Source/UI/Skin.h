#pragma once

#include <JuceHeader.h>

namespace Skin
{
    /** "VCO1 Octave" -> "vco1_octave". */
    juce::String normalise (const juce::String& name);

    /** Image asset from BinaryData or Resources folder; invalid image if absent. */
    juce::Image find (const juce::String& assetName);
    bool has (const juce::String& assetName);

    /** Clear image cache (for runtime hot-reloading). */
    void clearCache();

    /** Draws assetName scaled to bounds, else a rounded rect vector fallback. */
    void drawPanel (juce::Graphics& g, const juce::String& assetName,
                    juce::Rectangle<float> bounds, juce::Colour fallbackColour,
                    float cornerRadius);

    /**
     * Renders a rotary slider from a vertical/horizontal 3D filmstrip.
     * Returns true if a valid 3D filmstrip was found and drawn; false otherwise (fallback to vector).
     */
    bool drawFilmstrip (juce::Graphics& g, const juce::String& knobName,
                        float normalizedValue, juce::Rectangle<float> bounds);

    /**
     * Renders a multi-state button from a 3D sprite sheet or discrete state images.
     * Returns true if a valid 3D sprite was found and drawn; false otherwise (fallback to vector).
     */
    bool drawButtonSprite (juce::Graphics& g, const juce::String& buttonName, const juce::Button& button,
                           juce::Rectangle<float> bounds, bool isHighlighted, bool isDown);

    /**
     * Renders a step sequencer button (32 steps) from a 3D sprite sheet.
     * Returns true if drawn; false otherwise (fallback to vector).
     */
    bool drawStepButton (juce::Graphics& g, const juce::Button& button,
                         juce::Rectangle<float> bounds, bool isHighlighted, bool isDown);

    /**
     * Renders an octave lever switch from a 3D sprite sheet (5 positions).
     * Returns true if drawn; false otherwise (fallback to vector).
     */
    bool drawLever (juce::Graphics& g, const juce::String& leverName,
                    int positionIndex, int numPositions, juce::Rectangle<float> bounds);

    /**
     * Renders an optional 3D glass bezel / reflection overlay on top of the OLED display.
     */
    void drawScreenOverlay (juce::Graphics& g, juce::Rectangle<float> bounds);
}
