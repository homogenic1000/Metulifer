#include "Typography.h"

namespace Typography
{
    const juce::String& getNeoGrotesqueName()
    {
        static const juce::String family = []() -> juce::String
        {
            const auto families = juce::Font::findAllTypefaceNames();

            // 1. Inter - world reference for digital UI & synth neo-grotesque
            if (families.contains ("Inter", true))
                return "Inter";

            // 2. Apple San Francisco - macOS native precision neo-grotesque
            if (families.contains ("SF Pro Text", true))
                return "SF Pro Text";
            if (families.contains ("SF Pro", true))
                return "SF Pro";
            if (families.contains (".AppleSystemUIFont", true))
                return ".AppleSystemUIFont";

            // 3. Helvetica Neue - Swiss modernist archetype
            if (families.contains ("Helvetica Neue", true))
                return "Helvetica Neue";

            if (families.contains ("Helvetica", true))
                return "Helvetica";

            return juce::Font::getDefaultSansSerifFontName();
        }();

        return family;
    }

    bool isNeoGrotesqueActive()
    {
        const auto& name = getNeoGrotesqueName();
        return name.containsIgnoreCase ("Inter")
            || name.containsIgnoreCase ("SF Pro")
            || name.containsIgnoreCase ("Helvetica");
    }

    juce::FontOptions getOptions (float size, int styleFlags)
    {
        return juce::FontOptions (getNeoGrotesqueName(), size, styleFlags);
    }

    juce::Font getFont (float size, int styleFlags)
    {
        return juce::Font (getOptions (size, styleFlags));
    }
}
