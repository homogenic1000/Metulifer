#include "Skin.h"

#if __has_include ("BinaryData.h")
    #include "BinaryData.h"
    #define METULIFER_HAS_BINARY_DATA 1
#else
    #define METULIFER_HAS_BINARY_DATA 0
#endif

#include <map>
#include <cctype>

namespace Skin
{
    static std::map<juce::String, juce::Image> imageCache;
    static bool scanned = false;

    juce::String normalise (const juce::String& name)
    {
        const std::string raw = name.toStdString();
        std::string out;
        out.reserve (raw.size());

        for (char c : raw)
        {
            const auto uc = (unsigned char) c;
            out += std::isalnum (uc) != 0 ? (char) std::tolower (uc) : '_';
        }

        return juce::String (out);
    }

    static void scan()
    {
        if (scanned)
            return;

        scanned = true;

       #if METULIFER_HAS_BINARY_DATA
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const auto* resourceName = BinaryData::namedResourceList[i];
            int size = 0;
            const auto* data = BinaryData::getNamedResource (resourceName, size);

            if (data == nullptr)
                continue;

            auto img = juce::ImageCache::getFromMemory (data, (size_t) size);

            if (! img.isValid())
                continue;

            imageCache[normalise (resourceName)] = img;

            if (const auto* original = BinaryData::getNamedResourceOriginalFilename (resourceName))
                imageCache[normalise (juce::File::createFileWithoutExtension (original))] = img;
        }
       #endif
    }

    juce::Image find (const juce::String& assetName)
    {
        scan();

        const auto it = imageCache.find (normalise (assetName));
        return it != imageCache.end() ? it->second : juce::Image();
    }

    bool has (const juce::String& assetName)
    {
        return find (assetName).isValid();
    }

    void drawPanel (juce::Graphics& g, const juce::String& assetName,
                    juce::Rectangle<float> bounds, juce::Colour fallbackColour,
                    float cornerRadius)
    {
        if (auto img = find (assetName); img.isValid())
        {
            g.drawImage (img, bounds, juce::RectanglePlacement::stretchToFit, false);
            return;
        }

        // Panel drop shadow for separation against main background
        g.setColour (juce::Colour (0x12000000));
        g.fillRoundedRectangle (bounds.translated (0.0f, 1.5f), cornerRadius);

        // Subtle gradient fill for depth
        const auto topCol = fallbackColour.brighter (0.02f);
        const auto botCol = fallbackColour.darker (0.03f);
        juce::ColourGradient grad (topCol, bounds.getCentreX(), bounds.getY(),
                                   botCol, bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, cornerRadius);

        // Crisp border outline
        g.setColour (juce::Colour (0xffb4b9c2));
        g.drawRoundedRectangle (bounds, cornerRadius, 1.0f);

        // Subtle top highlight
        g.setColour (juce::Colour (0x45ffffff));
        g.drawRoundedRectangle (bounds.reduced (0.8f), juce::jmax (1.0f, cornerRadius - 0.8f), 0.8f);
    }
}
