#include "DisplayScreen.h"

DisplayScreen::DisplayScreen()
{
}

void DisplayScreen::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (juce::Colour (0xff1a1a1a));
    g.fillRoundedRectangle (bounds, 8.0f);

    g.setColour (juce::Colour (0xff3a7bd5));
    g.drawRoundedRectangle (bounds, 8.0f, 1.5f);

    auto area = getLocalBounds().reduced (14);

    g.setColour (juce::Colour (0xff6fe07a));
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("SEQ " + juce::String (sequenceNumber), area.removeFromTop (26), juce::Justification::centredLeft);

    auto paramArea = area.removeFromTop (24);
    if (paramText.isNotEmpty())
    {
        g.setColour (juce::Colour (0xff6fe07a));
        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        g.drawText (paramText, paramArea, juce::Justification::centredLeft);
    }

    auto signalArea = getLocalBounds().removeFromBottom (34).reduced (14, 8);
    g.setColour (juce::Colour (0xff2a2a2a));
    g.fillRoundedRectangle (signalArea.toFloat(), 4.0f);

    auto level = juce::jlimit (0.0f, 1.0f, rms);
    auto fill = signalArea.toFloat().reduced (2.0f);
    fill.setWidth (fill.getWidth() * level);
    g.setColour (juce::Colour (0xff6fe07a));
    g.fillRoundedRectangle (fill, 3.0f);
}

void DisplayScreen::setSequenceNumber (int index)
{
    sequenceNumber = juce::jmax (1, index);
    repaint();
}

void DisplayScreen::setParamText (const juce::String& text)
{
    paramText = text;
    repaint();
}

void DisplayScreen::setRmsLevel (float newRms)
{
    rms = newRms;
    repaint();
}
