#include "DisplayScreen.h"
#include "Skin.h"

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
        g.strokePath (wavePath, juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));

        // Waveform crisp core
        g.setColour (juce::Colour (0xff38bdf8));
        g.strokePath (wavePath, juce::PathStrokeType (1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));
    }

    Skin::drawScreenOverlay (g, getLocalBounds().toFloat());
}

void DisplayScreen::setSequenceNumber (int index)
{
    const int newSeq = juce::jmax (1, index);
    if (sequenceNumber != newSeq)
    {
        sequenceNumber = newSeq;
        repaint();
    }
}

void DisplayScreen::setParamText (const juce::String& text)
{
    if (paramText != text)
    {
        paramText = text;
        repaint();
    }
}

void DisplayScreen::setRmsLevel (float newRms)
{
    rms = newRms;
}

void DisplayScreen::updateMeterLevels (float inPeakL, float inPeakR, float /*inRmsL*/, float /*inRmsR*/)
{
    const float oldPeakL = meterPeakL;
    const float oldPeakR = meterPeakR;
    const float oldHoldL = peakHoldL;
    const float oldHoldR = peakHoldR;

    // Fast attack (instantaneous)
    if (inPeakL > meterPeakL)
        meterPeakL = inPeakL;
    else
        meterPeakL = juce::jmax (-80.0f, meterPeakL - 0.9f); // ~54 dB/s decay at 60 Hz

    if (inPeakR > meterPeakR)
        meterPeakR = inPeakR;
    else
        meterPeakR = juce::jmax (-80.0f, meterPeakR - 0.9f);

    // Peak Hold with 1s hold time (60 frames @ 60 Hz = 1.0 s)
    if (meterPeakL >= peakHoldL)
    {
        peakHoldL = meterPeakL;
        peakHoldTimerL = 60;
    }
    else
    {
        if (peakHoldTimerL > 0)
            --peakHoldTimerL;
        else
            peakHoldL = juce::jmax (-80.0f, peakHoldL - 0.7f);
    }

    if (meterPeakR >= peakHoldR)
    {
        peakHoldR = meterPeakR;
        peakHoldTimerR = 60;
    }
    else
    {
        if (peakHoldTimerR > 0)
            --peakHoldTimerR;
        else
            peakHoldR = juce::jmax (-80.0f, peakHoldR - 0.7f);
    }

    lastDbReadout = juce::jmax (meterPeakL, meterPeakR);

    // Meters are only considered dirty if not resting at silence (-80 dB)
    // or if the visible value shifted
    const bool atSilence = (meterPeakL <= -80.0f && meterPeakR <= -80.0f
                            && peakHoldL <= -80.0f && peakHoldR <= -80.0f);
    const bool wasAtSilence = (oldPeakL <= -80.0f && oldPeakR <= -80.0f
                               && oldHoldL <= -80.0f && oldHoldR <= -80.0f);

    if (! (atSilence && wasAtSilence))
    {
        if (std::abs (oldPeakL - meterPeakL) > 0.05f || std::abs (oldPeakR - meterPeakR) > 0.05f
            || std::abs (oldHoldL - peakHoldL) > 0.05f || std::abs (oldHoldR - peakHoldR) > 0.05f)
        {
            meterDirty = true;
        }
    }
}

void DisplayScreen::pushScopeSamples (const float* samples, int numSamples)
{
    bool scopeChanged = false;

    if (samples != nullptr && numSamples > 0)
    {
        // Detect signal peak to check for silence or active sound
        float maxAmp = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            const float a = std::abs (samples[i]);
            if (a > maxAmp)
                maxAmp = a;
        }

        if (maxAmp < 0.005f)
        {
            if (! scopeIsFlat)
            {
                // Smoothly decay to flat line at zero when silent
                float maxRemaining = 0.0f;
                for (int i = 0; i < numScopePoints; ++i)
                {
                    scopePoints[i] *= 0.75f;
                    maxRemaining = juce::jmax (maxRemaining, std::abs (scopePoints[i]));
                }

                if (maxRemaining < 0.001f)
                {
                    for (int i = 0; i < numScopePoints; ++i)
                        scopePoints[i] = 0.0f;
                    scopeIsFlat = true;
                }
                scopeChanged = true;
            }
        }
        else
        {
            scopeIsFlat = false;
            scopeChanged = true;

            // Trigger detection: find the first rising zero-crossing
            const int searchRange = juce::jmin (numSamples / 2, 256);
            int triggerIdx = -1;
            for (int i = 1; i < searchRange; ++i)
            {
                if (samples[i - 1] <= 0.0f && samples[i] > 0.0f)
                {
                    triggerIdx = i;
                    break;
                }
            }

            if (triggerIdx < 0)
                triggerIdx = 0;

            // Estimate fundamental period by finding the second rising zero-crossing
            int period = 0;
            const int minPeriod = 6;
            for (int i = triggerIdx + minPeriod; i < numSamples - 1; ++i)
            {
                if (samples[i - 1] <= 0.0f && samples[i] > 0.0f)
                {
                    period = i - triggerIdx;
                    break;
                }
            }

            // Choose window to show 2 cycles if period detected, or a reasonable window
            int samplesToShow;
            if (period > 0)
            {
                const int targetCycles = (period < 90) ? 3 : 2;
                samplesToShow = juce::jlimit (32, numSamples - triggerIdx, targetCycles * period);
            }
            else
            {
                samplesToShow = juce::jlimit (32, numSamples - triggerIdx, 256);
            }

            // Auto-gain so the waveform comfortably fills ~82% of vertical range
            const float autoGain = juce::jlimit (1.0f, 4.0f, 0.82f / maxAmp);

            // Resample to numScopePoints (128) using linear interpolation
            const float step = (float) (samplesToShow - 1) / (float) (numScopePoints - 1);
            for (int i = 0; i < numScopePoints; ++i)
            {
                const float srcPos = (float) triggerIdx + (float) i * step;
                const int i0 = juce::jlimit (0, numSamples - 1, (int) srcPos);
                const int i1 = juce::jlimit (0, numSamples - 1, i0 + 1);
                const float frac = srcPos - (float) i0;
                const float rawSample = (samples[i0] + frac * (samples[i1] - samples[i0])) * autoGain;

                // Immediate zero-latency sample update for instantaneous rhythmic response
                scopePoints[i] = rawSample;
            }
        }
    }

    // Coalesce meter and scope repaints into a single repaint call
    if (meterDirty || scopeChanged)
    {
        meterDirty = false;
        repaint();
    }
}
