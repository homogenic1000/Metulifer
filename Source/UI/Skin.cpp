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

    static juce::Image loadFromDisk (const juce::String& cleanName)
    {
        static const juce::StringArray extensions { ".png", "@2x.png", ".jpg", ".jpeg" };

        const auto dirs = {
            juce::File ("/Users/mathoodelessert/juice/Metulifer/Resources"),
            juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getSiblingFile ("Resources"),
            juce::File::getSpecialLocation (juce::File::currentApplicationFile).getChildFile ("Contents/Resources"),
            juce::File::getCurrentWorkingDirectory().getChildFile ("Resources")
        };

        for (const auto& dir : dirs)
        {
            if (! dir.isDirectory())
                continue;

            for (const auto& ext : extensions)
            {
                auto file = dir.getChildFile (cleanName + ext);
                if (file.existsAsFile())
                {
                    auto img = juce::ImageFileFormat::loadFrom (file);
                    if (img.isValid())
                        return img;
                }
            }
        }

        return {};
    }

    void clearCache()
    {
        imageCache.clear();
        scanned = false;
    }

    juce::Image find (const juce::String& assetName)
    {
        scan();

        const auto key = normalise (assetName);
        const auto it = imageCache.find (key);
        if (it != imageCache.end())
            return it->second;

        auto diskImg = loadFromDisk (key);
        if (diskImg.isValid())
        {
            imageCache[key] = diskImg;
            return diskImg;
        }

        return {};
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

    bool drawFilmstrip (juce::Graphics& g, const juce::String& knobName,
                        float normalizedValue, juce::Rectangle<float> bounds)
    {
        const auto norm = normalise (knobName);
        juce::Image filmstrip;

        // 1. Specific knob with _filmstrip suffix
        filmstrip = find (norm + "_filmstrip");
        if (! filmstrip.isValid())
            filmstrip = find (norm);

        // 2. Generic size-based fallback
        if (! filmstrip.isValid())
        {
            const float maxDim = juce::jmax (bounds.getWidth(), bounds.getHeight());
            if (maxDim >= 70.0f)
                filmstrip = find ("knob_big_filmstrip");
            else if (maxDim >= 40.0f)
                filmstrip = find ("knob_med_filmstrip");
            else
                filmstrip = find ("knob_small_filmstrip");
        }

        if (! filmstrip.isValid())
            return false;

        const int imgW = filmstrip.getWidth();
        const int imgH = filmstrip.getHeight();

        if (imgW <= 0 || imgH <= 0)
            return false;

        const bool isVertical = (imgH >= imgW);
        const int numFrames = isVertical ? (imgH / imgW) : (imgW / imgH);

        if (numFrames <= 1)
        {
            g.drawImage (filmstrip, bounds, juce::RectanglePlacement::centred, false);
            return true;
        }

        const int frameIndex = juce::jlimit (0, numFrames - 1,
                                             juce::roundToInt (normalizedValue * (float) (numFrames - 1)));

        const int frameW = isVertical ? imgW : (imgW / numFrames);
        const int frameH = isVertical ? (imgH / numFrames) : imgH;
        const int srcX   = isVertical ? 0 : (frameIndex * frameW);
        const int srcY   = isVertical ? (frameIndex * frameH) : 0;

        juce::Graphics::ScopedSaveState s (g);
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (filmstrip,
                     juce::roundToInt (bounds.getX()), juce::roundToInt (bounds.getY()),
                     juce::roundToInt (bounds.getWidth()), juce::roundToInt (bounds.getHeight()),
                     srcX, srcY, frameW, frameH);

        return true;
    }

    bool drawButtonSprite (juce::Graphics& g, const juce::String& buttonName, const juce::Button& button,
                           juce::Rectangle<float> bounds, bool isHighlighted, bool isDown)
    {
        const auto norm = normalise (buttonName);
        const bool isOn = button.getToggleState();

        // 1. Multi-state filmstrip strip: btn_<name>_filmstrip
        auto strip = find ("btn_" + norm + "_filmstrip");
        if (! strip.isValid())
            strip = find (norm + "_filmstrip");

        if (strip.isValid())
        {
            const int imgW = strip.getWidth();
            const int imgH = strip.getHeight();
            const int numFrames = imgH / imgW;

            int frameIdx = 0;
            if (numFrames >= 6)
            {
                // 0: Off normal, 1: Off hover, 2: Off down
                // 3: On normal,  4: On hover,  5: On down
                frameIdx = isOn ? (isDown ? 5 : (isHighlighted ? 4 : 3))
                                : (isDown ? 2 : (isHighlighted ? 1 : 0));
            }
            else if (numFrames >= 4)
            {
                // 0: Off normal, 1: Off hover/down, 2: On normal, 3: On hover/down
                frameIdx = isOn ? (isDown || isHighlighted ? 3 : 2)
                                : (isDown || isHighlighted ? 1 : 0);
            }
            else if (numFrames >= 2)
            {
                // 0: Off, 1: On
                frameIdx = isOn ? 1 : 0;
            }

            const int frameH = imgH / numFrames;
            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (strip,
                         juce::roundToInt (bounds.getX()), juce::roundToInt (bounds.getY()),
                         juce::roundToInt (bounds.getWidth()), juce::roundToInt (bounds.getHeight()),
                         0, frameIdx * frameH, imgW, frameH);
            return true;
        }

        // 2. Discrete state images
        juce::String stateSuffix;
        if (isOn)
            stateSuffix = isDown ? "_on_down" : (isHighlighted ? "_on_hover" : "_on");
        else
            stateSuffix = isDown ? "_down" : (isHighlighted ? "_hover" : "_off");

        auto img = find ("btn_" + norm + stateSuffix);
        if (! img.isValid() && isOn)
            img = find ("btn_" + norm + "_on");
        if (! img.isValid() && ! isOn)
            img = find ("btn_" + norm + "_off");
        if (! img.isValid())
            img = find ("btn_" + norm);

        if (img.isValid())
        {
            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (img, bounds, juce::RectanglePlacement::centred, false);
            return true;
        }

        return false;
    }

    bool drawStepButton (juce::Graphics& g, const juce::Button& button,
                         juce::Rectangle<float> bounds, bool isHighlighted, bool isDown)
    {
        const int stepNum = button.getButtonText().substring (5).getIntValue();
        const bool isDownbeat = ((stepNum - 1) % 4 == 0);
        const bool isOn = button.getToggleState();

        auto strip = find ("step_button_filmstrip");
        if (strip.isValid())
        {
            const int imgW = strip.getWidth();
            const int imgH = strip.getHeight();
            const int numFrames = imgH / imgW;

            int frameIdx = 0;
            if (numFrames >= 6)
            {
                frameIdx = isOn ? (isDown ? 5 : (isHighlighted ? 4 : 3))
                                : (isDown ? 2 : (isHighlighted ? 1 : 0));
            }
            else if (numFrames >= 2)
            {
                frameIdx = isOn ? 1 : 0;
            }

            const int frameH = imgH / numFrames;
            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (strip,
                         juce::roundToInt (bounds.getX()), juce::roundToInt (bounds.getY()),
                         juce::roundToInt (bounds.getWidth()), juce::roundToInt (bounds.getHeight()),
                         0, frameIdx * frameH, imgW, frameH);
            return true;
        }

        juce::Image img;
        if (isOn)
            img = find (isDown ? "step_on_down" : (isHighlighted ? "step_on_hover" : "step_on"));
        else if (isDownbeat)
            img = find (isDown ? "step_downbeat_down" : (isHighlighted ? "step_downbeat_hover" : "step_downbeat"));

        if (! img.isValid() && ! isOn)
            img = find (isDown ? "step_off_down" : (isHighlighted ? "step_off_hover" : "step_off"));

        if (img.isValid())
        {
            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (img, bounds, juce::RectanglePlacement::centred, false);
            return true;
        }

        return false;
    }

    bool drawLever (juce::Graphics& g, const juce::String& leverName, int positionIndex, int numPositions,
                    juce::Rectangle<float> bounds)
    {
        const auto norm = normalise (leverName);

        auto strip = find ("lever_" + norm + "_filmstrip");
        if (! strip.isValid())
            strip = find ("lever_octave_filmstrip");

        if (strip.isValid())
        {
            const int imgW = strip.getWidth();
            const int imgH = strip.getHeight();
            const int frames = imgH / imgW;

            const int frameIdx = juce::jlimit (0, frames - 1, positionIndex);
            const int frameH = imgH / frames;

            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (strip,
                         juce::roundToInt (bounds.getX()), juce::roundToInt (bounds.getY()),
                         juce::roundToInt (bounds.getWidth()), juce::roundToInt (bounds.getHeight()),
                         0, frameIdx * frameH, imgW, frameH);
            return true;
        }

        auto img = find ("lever_" + norm + "_" + juce::String (positionIndex));
        if (! img.isValid())
            img = find ("lever_octave_" + juce::String (positionIndex));

        if (img.isValid())
        {
            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (img, bounds, juce::RectanglePlacement::centred, false);
            return true;
        }

        return false;
    }

    void drawScreenOverlay (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        auto glass = find ("screen_glass_overlay");
        if (! glass.isValid())
            glass = find ("screen_glass");

        if (glass.isValid())
        {
            juce::Graphics::ScopedSaveState s (g);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (glass, bounds, juce::RectanglePlacement::stretchToFit, false);
        }
    }
}
