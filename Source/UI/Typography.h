#pragma once

#include <JuceHeader.h>

namespace Typography
{
    /** Returns the detected Neo-Grotesque font family name.
     *  Hierarchy: "Inter" -> "SF Pro Text" -> "SF Pro" -> "Helvetica Neue" -> default sans-serif.
     */
    const juce::String& getNeoGrotesqueName();

    /** Checks whether a true Neo-Grotesque typeface (Inter, SF Pro, Helvetica Neue) is active. */
    bool isNeoGrotesqueActive();

    /** Creates a FontOptions pre-configured with the Neo-Grotesque typeface and specified size/style. */
    juce::FontOptions getOptions (float size, int styleFlags = juce::Font::plain);

    /** Creates a Font pre-configured with the Neo-Grotesque typeface and specified size/style. */
    juce::Font getFont (float size, int styleFlags = juce::Font::plain);

    /** Convenience helper for Bold Neo-Grotesque Font */
    inline juce::Font getBold (float size)
    {
        return getFont (size, juce::Font::bold);
    }

    /** Convenience helper for Section Headings */
    inline juce::Font getHeading (float size = 15.0f)
    {
        return getFont (size, juce::Font::bold);
    }

    /** Convenience helper for Subheadings / Module Titles */
    inline juce::Font getSubheading (float size = 12.0f)
    {
        return getFont (size, juce::Font::bold);
    }

    /** Convenience helper for Parameter Labels */
    inline juce::Font getLabel (float size = 10.0f)
    {
        return getFont (size, juce::Font::plain);
    }

    /** Convenience helper for Bold Parameter Labels */
    inline juce::Font getBoldLabel (float size = 10.0f)
    {
        return getFont (size, juce::Font::bold);
    }

    /** Convenience helper for Display & Numerical Readouts */
    inline juce::Font getNumeric (float size = 11.0f)
    {
        return getFont (size, juce::Font::bold);
    }

    /** Convenience helper for Buttons */
    inline juce::Font getButton (float size = 11.0f)
    {
        return getFont (size, juce::Font::bold);
    }
}
