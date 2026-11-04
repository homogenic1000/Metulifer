#include "DisplayScreen.h"

DisplayScreen::DisplayScreen()
{
}

void DisplayScreen::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    // Drop shadow
    g.setColour (juce::Colour (0x24000000));
    g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), 8.0f);

    // OLED Dark Glass Screen
    g.setColour (juce::Colour (0xff0a0d14));
    g.fillRoundedRectangle (bounds, 8.0f);

    // High-tech Cyber Blue / Steel border with subtle glow
    g.setColour (juce::Colour (0xff1e3a8a));
    g.drawRoundedRectangle (bounds, 8.0f, 1.5f);
    g.setColour (juce::Colour (0x2560a5fa));
    g.drawRoundedRectangle (bounds.reduced (1.0f), 7.0f, 1.0f);

    auto inner = bounds.reduced (10.0f);

    // --- Header Strip ---
    auto headerArea = inner.removeFromTop (22.0f);

    // Sequence Badge
    auto badgeArea = headerArea.removeFromLeft (64.0f);
    g.setColour (juce::Colour (0xff14291f));
    g.fillRoundedRectangle (badgeArea, 4.0f);
    g.setColour (juce::Colour (0xff22c55e));
    g.drawRoundedRectangle (badgeArea, 4.0f, 1.0f);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("SEQ " + juce::String (sequenceNumber), badgeArea, juce::Justification::centred);

    // Mode Tag (Right)
    auto modeTagArea = headerArea.removeFromRight (74.0f);
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    if (paramText.isNotEmpty())
    {
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawText ("EDIT", modeTagArea, juce::Justification::centredRight);
    }
    else
    {
        // Pulsing / live scope indicator
        g.setColour (juce::Colour (0xff38bdf8));
        g.fillEllipse (modeTagArea.getRight() - 50.0f, modeTagArea.getCentreY() - 3.0f, 6.0f, 6.0f);
        g.setColour (juce::Colour (0xff94a3b8));
        g.drawText ("SCOPE", modeTagArea.removeFromRight (40.0f), juce::Justification::centredRight);
    }

    // --- Bottom RMS Bar ---
    auto meterArea = inner.removeFromBottom (10.0f);
    g.setColour (juce::Colour (0xff1e2433));
    g.fillRoundedRectangle (meterArea, 3.0f);

    const float level = juce::jlimit (0.0f, 1.0f, rms);
    if (level > 0.001f)
    {
        auto fill = meterArea;
        fill.setWidth (fill.getWidth() * level);
        juce::ColourGradient grad (juce::Colour (0xff10b981), fill.getX(), fill.getY(),
                                   level > 0.8f ? juce::Colour (0xffef4444) : juce::Colour (0xff38bdf8),
                                   fill.getRight(), fill.getY(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (fill, 3.0f);
    }

    inner.removeFromBottom (6.0f); // Spacing between scope/param and meter

    // --- Main Display Area (Parameter Edit vs Real-time Oscilloscope) ---
    auto mainArea = inner;

    if (paramText.isNotEmpty())
    {
        // Parameter Edit Mode
        g.setColour (juce::Colour (0x1238bdf8));
        g.fillRoundedRectangle (mainArea, 5.0f);
        g.setColour (juce::Colour (0x3038bdf8));
        g.drawRoundedRectangle (mainArea, 5.0f, 1.0f);

        g.setColour (juce::Colour (0xffe2e8f0));
        g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
        g.drawText (paramText, mainArea.reduced (8.0f), juce::Justification::centred, true);
    }
    else
    {
        // Oscilloscope Mode
        // Graticule grid
        g.setColour (juce::Colour (0x1538bdf8));
        const float midY = mainArea.getCentreY();
        g.drawHorizontalLine (juce::roundToInt (midY), mainArea.getX(), mainArea.getRight());
        g.drawHorizontalLine (juce::roundToInt (mainArea.getY() + mainArea.getHeight() * 0.25f), mainArea.getX(), mainArea.getRight());
        g.drawHorizontalLine (juce::roundToInt (mainArea.getY() + mainArea.getHeight() * 0.75f), mainArea.getX(), mainArea.getRight());

        for (float xRatio : { 0.25f, 0.5f, 0.75f })
        {
            const float gx = mainArea.getX() + mainArea.getWidth() * xRatio;
            g.drawVerticalLine (juce::roundToInt (gx), mainArea.getY(), mainArea.getBottom());
        }

        // Draw waveform path
        juce::Path wavePath;
        const float w = mainArea.getWidth();
        const float h = mainArea.getHeight() * 0.44f; // Max amplitude scaling
        const float cx = mainArea.getX();

        for (int i = 0; i < numScopePoints; ++i)
        {
            const float px = cx + (float) i * (w / (float) (numScopePoints - 1));
            const float py = midY - (juce::jlimit (-1.0f, 1.0f, scopePoints[i]) * h);

            if (i == 0)
                wavePath.startNewSubPath (px, py);
            else
                wavePath.lineTo (px, py);
        }

        // Waveform outer glow
        g.setColour (juce::Colour (0x3538bdf8));
        g.strokePath (wavePath, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Waveform crisp core
        g.setColour (juce::Colour (0xff38bdf8));
        g.strokePath (wavePath, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
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

void DisplayScreen::pushScopeSamples (const float* samples, int numSamples)
{
    if (samples == nullptr || numSamples <= 0)
        return;

    const float step = (float) numSamples / (float) numScopePoints;
    for (int i = 0; i < numScopePoints; ++i)
    {
        const int idx = juce::jlimit (0, numSamples - 1, juce::roundToInt ((float) i * step));
        scopePoints[i] = samples[idx];
    }

    repaint();
}
