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

    // --- Bottom Accurate dB Meter ---
    auto meterArea = inner.removeFromBottom (20.0f);

    // Right side: Exact numerical dB readout (54 px)
    auto readoutArea = meterArea.removeFromRight (54.0f);
    g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
    if (lastDbReadout >= -0.05f)
    {
        g.setColour (juce::Colour (0xffef4444));
        g.drawText ("CLIP", readoutArea, juce::Justification::centredRight);
    }
    else if (lastDbReadout <= -48.0f)
    {
        g.setColour (juce::Colour (0xff64748b));
        g.drawText ("-inf dB", readoutArea, juce::Justification::centredRight);
    }
    else
    {
        g.setColour (juce::Colour (0xffcbd5e1));
        g.drawText (juce::String (lastDbReadout, 1) + " dB", readoutArea, juce::Justification::centredRight);
    }

    meterArea.removeFromRight (8.0f); // gap between bars and readout

    // Left side: L and R channels
    const float labelWidth = 10.0f;
    auto barsArea = meterArea;
    barsArea.removeFromLeft (labelWidth);

    auto barL = barsArea.removeFromTop (4.0f);
    barsArea.removeFromTop (2.0f);
    auto barR = barsArea.removeFromTop (4.0f);
    barsArea.removeFromTop (2.0f);

    // Labels "L" and "R"
    g.setFont (juce::FontOptions (7.5f, juce::Font::bold));
    g.setColour (juce::Colour (0xff64748b));
    g.drawText ("L", meterArea.getX(), barL.getY() - 1.0f, labelWidth, 6.0f, juce::Justification::centredLeft);
    g.drawText ("R", meterArea.getX(), barR.getY() - 1.0f, labelWidth, 6.0f, juce::Justification::centredLeft);

    // Track background
    g.setColour (juce::Colour (0xff141a24));
    g.fillRoundedRectangle (barL, 1.5f);
    g.fillRoundedRectangle (barR, 1.5f);

    // Scale mapping helper: -48 dB to +3 dB
    auto dbToNorm = [] (float db) -> float
    {
        const float clamped = juce::jlimit (-48.0f, 3.0f, db);
        return (clamped + 48.0f) / 51.0f; // 0 dB is at 48/51 = 0.941
    };

    auto drawMeterBar = [&] (juce::Rectangle<float> barRect, float peakDb, float holdDb)
    {
        const float norm = dbToNorm (peakDb);
        if (norm > 0.005f)
        {
            auto fill = barRect;
            fill.setWidth (barRect.getWidth() * norm);

            // Multistage gradient: green (-48 to -12) -> yellow (-12 to -3) -> orange (-3 to 0) -> red (>0)
            juce::Colour endCol = juce::Colour (0xff10b981);
            if (peakDb > 0.0f)
                endCol = juce::Colour (0xffef4444);
            else if (peakDb > -3.0f)
                endCol = juce::Colour (0xfff97316);
            else if (peakDb > -12.0f)
                endCol = juce::Colour (0xffeab308);

            juce::ColourGradient grad (juce::Colour (0xff10b981), fill.getX(), fill.getY(),
                                       endCol, fill.getRight(), fill.getY(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (fill, 1.5f);
        }

        // Peak Hold indicator tick
        const float holdNorm = dbToNorm (holdDb);
        if (holdNorm > 0.01f)
        {
            const float tickX = barRect.getX() + barRect.getWidth() * holdNorm;
            g.setColour (holdDb >= 0.0f ? juce::Colour (0xffef4444) : juce::Colours::white);
            g.drawVerticalLine (juce::roundToInt (tickX), barRect.getY(), barRect.getBottom());
        }
    };

    drawMeterBar (barL, meterPeakL, peakHoldL);
    drawMeterBar (barR, meterPeakR, peakHoldR);

    // Scale ticks and numbers: -48, -24, -12, -6, 0, +3
    static const float scaleValues[] = { -48.0f, -24.0f, -12.0f, -6.0f, 0.0f, 3.0f };
    static const char* const scaleLabels[] = { "-48", "-24", "-12", "-6", "0", "+3" };

    g.setFont (juce::FontOptions (7.0f, juce::Font::plain));
    for (int i = 0; i < 6; ++i)
    {
        const float tx = barL.getX() + barL.getWidth() * dbToNorm (scaleValues[i]);
        g.setColour (scaleValues[i] >= 0.0f ? juce::Colour (0xffef4444) : juce::Colour (0xff475569));
        g.drawVerticalLine (juce::roundToInt (tx), barR.getBottom() + 1.0f, barR.getBottom() + 3.0f);
        g.drawText (scaleLabels[i], juce::roundToInt (tx) - 10, juce::roundToInt (barR.getBottom() + 2.0f), 20, 8, juce::Justification::centred);
    }

    inner.removeFromBottom (4.0f); // Spacing between scope/param and meter

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

void DisplayScreen::updateMeterLevels (float inPeakL, float inPeakR, float /*inRmsL*/, float /*inRmsR*/)
{
    // Fast attack (instantaneous)
    if (inPeakL > meterPeakL)
        meterPeakL = inPeakL;
    else
        meterPeakL = juce::jmax (-80.0f, meterPeakL - 1.8f);

    if (inPeakR > meterPeakR)
        meterPeakR = inPeakR;
    else
        meterPeakR = juce::jmax (-80.0f, meterPeakR - 1.8f);

    // Peak Hold with 1s hold time
    if (meterPeakL >= peakHoldL)
    {
        peakHoldL = meterPeakL;
        peakHoldTimerL = 30; // 30 frames @ 30 Hz = 1.0 s
    }
    else
    {
        if (peakHoldTimerL > 0)
            --peakHoldTimerL;
        else
            peakHoldL = juce::jmax (-80.0f, peakHoldL - 1.2f);
    }

    if (meterPeakR >= peakHoldR)
    {
        peakHoldR = meterPeakR;
        peakHoldTimerR = 30;
    }
    else
    {
        if (peakHoldTimerR > 0)
            --peakHoldTimerR;
        else
            peakHoldR = juce::jmax (-80.0f, peakHoldR - 1.2f);
    }

    lastDbReadout = juce::jmax (meterPeakL, meterPeakR);
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
