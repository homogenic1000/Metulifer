#include "MetuliferLookAndFeel.h"
#include "Skin.h"

MetuliferLookAndFeel::MetuliferLookAndFeel() = default;

void MetuliferLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y,
                                             int width, int height, float sliderPos,
                                             float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider& slider)
{
    auto img = Skin::find ("knob_" + slider.getName());

    if (img.isValid() && img.getHeight() > 0)
    {
        const int frameH = img.getHeight();
        const int frames = juce::jmax (1, img.getWidth() / frameH);
        const int frame = juce::jlimit (0, frames - 1,
                                        juce::roundToInt (sliderPos * (float) (frames - 1)));

        g.drawImage (img, x, y, width, height, frame * frameH, 0, frameH, frameH, true);
        return;
    }

    LookAndFeel_V4::drawRotarySlider (g, x, y, width, height, sliderPos,
                                      rotaryStartAngle, rotaryEndAngle, slider);
}

juce::Image MetuliferLookAndFeel::findButtonImage (const juce::Button& button)
{
    const auto stem = "btn_" + button.getButtonText();

    if (auto img = Skin::find (stem + (button.getToggleState() ? "_on" : "_off")); img.isValid())
        return img;

    return Skin::find (stem);
}

void MetuliferLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour& backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    if (auto img = findButtonImage (button); img.isValid())
    {
        g.drawImage (img, button.getLocalBounds().toFloat(),
                     juce::RectanglePlacement::centred, false);
        return;
    }

    LookAndFeel_V4::drawButtonBackground (g, button, backgroundColour,
                                          shouldDrawButtonAsHighlighted,
                                          shouldDrawButtonAsDown);
}

void MetuliferLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{
    if (findButtonImage (button).isValid())
        return;

    LookAndFeel_V4::drawButtonText (g, button, shouldDrawButtonAsHighlighted,
                                    shouldDrawButtonAsDown);
}
