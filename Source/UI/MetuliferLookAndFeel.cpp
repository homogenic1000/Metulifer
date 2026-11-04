#include "MetuliferLookAndFeel.h"
#include "Skin.h"
#include <cmath>

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

    drawHardwareKnob (g, (float) x, (float) y, (float) width, (float) height,
                      sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
}

void MetuliferLookAndFeel::drawHardwareKnob (juce::Graphics& g, float x, float y,
                                             float width, float height, float sliderPos,
                                             float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider& slider)
{
    const float size = juce::jmin (width, height);
    if (size < 10.0f)
        return;

    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.5f;
    const float r = (size * 0.5f) - 2.5f;

    // Track arc
    const float trackR = r - 1.5f;
    const float trackThickness = juce::jlimit (2.5f, 6.0f, size * 0.08f);

    juce::Path bgTrack;
    bgTrack.addCentredArc (cx, cy, trackR, trackR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff1e2229));
    g.strokePath (bgTrack, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float currentAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    if (sliderPos > 0.005f)
    {
        juce::Path activeTrack;
        activeTrack.addCentredArc (cx, cy, trackR, trackR, 0.0f, rotaryStartAngle, currentAngle, true);

        juce::Colour activeColor (0xff2563eb); // Default electric blue
        const auto name = slider.getName().toLowerCase();
        if (name.contains ("tempo"))
            activeColor = juce::Colour (0xffff7a1a); // Amber
        else if (name.contains ("reverb"))
            activeColor = juce::Colour (0xffff9f1c); // Warm amber
        else if (name.contains ("cutoff"))
            activeColor = juce::Colour (0xff06b6d4); // Cyan
        else if (name.contains ("mix"))
            activeColor = juce::Colour (0xffa855f7); // Purple
        else if (name.contains ("volume"))
            activeColor = juce::Colour (0xff38bdf8); // Sky blue
        else if (name.contains ("length"))
            activeColor = juce::Colour (0xfff59e0b); // Golden

        if (! slider.isEnabled())
            activeColor = activeColor.withAlpha (0.35f);

        g.setColour (activeColor);
        g.strokePath (activeTrack, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Knob Cap Body
    const float capR = r * 0.76f;
    if (capR > 3.0f)
    {
        // Drop shadow
        g.setColour (juce::Colour (0x35000000));
        g.fillEllipse (cx - capR, cy - capR + 1.5f, capR * 2.0f, capR * 2.0f);

        // Metallic dark face
        juce::ColourGradient capGrad (juce::Colour (0xff373c46), cx, cy - capR,
                                      juce::Colour (0xff1c1e24), cx, cy + capR, false);
        g.setGradientFill (capGrad);
        g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

        // Bevel rim
        const bool isHovered = slider.isMouseOverOrDragging();
        juce::Colour rimCol = isHovered ? juce::Colour (0xff60a5fa) : juce::Colour (0xff4b5563);
        g.setColour (rimCol);
        g.drawEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f, 1.2f);

        // Center hub inset
        const float hubR = capR * 0.32f;
        g.setColour (juce::Colour (0xff14161b));
        g.fillEllipse (cx - hubR, cy - hubR, hubR * 2.0f, hubR * 2.0f);
        g.setColour (juce::Colour (0x25ffffff));
        g.drawEllipse (cx - hubR, cy - hubR, hubR * 2.0f, hubR * 2.0f, 0.8f);

        // Indicator Needle
        const float angleOffset = -juce::MathConstants<float>::halfPi;
        const float cosA = std::cos (currentAngle + angleOffset);
        const float sinA = std::sin (currentAngle + angleOffset);

        const float pStart = hubR + 1.0f;
        const float pEnd = capR - 2.0f;
        const float needleW = juce::jlimit (2.0f, 4.0f, size * 0.05f);

        g.setColour (isHovered ? juce::Colours::white : juce::Colour (0xffe2e8f0));
        g.drawLine (cx + cosA * pStart, cy + sinA * pStart,
                    cx + cosA * pEnd, cy + sinA * pEnd,
                    needleW);
    }
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

    const auto text = button.getButtonText();

    if (text.startsWith ("step "))
    {
        drawStepButtonBackground (g, button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        return;
    }

    drawStandardButtonBackground (g, button, backgroundColour, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void MetuliferLookAndFeel::drawStepButtonBackground (juce::Graphics& g, juce::Button& button,
                                                     bool isHighlighted, bool isDown)
{
    const int stepNum = button.getButtonText().substring (5).getIntValue();
    const bool isDownbeat = ((stepNum - 1) % 4 == 0);
    const bool isOn = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const float cr = 3.0f;

    if (isOn)
    {
        // Glowing Amber / Orange when ON
        juce::Colour topCol = isHighlighted ? juce::Colour (0xffffa04d)
                            : isDown        ? juce::Colour (0xffd84315)
                                            : juce::Colour (0xffff8533);
        juce::Colour botCol = isHighlighted ? juce::Colour (0xfff57c00)
                            : isDown        ? juce::Colour (0xffbf360c)
                                            : juce::Colour (0xffe65100);

        juce::ColourGradient grad (topCol, bounds.getCentreX(), bounds.getY(),
                                   botCol, bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, cr);

        // Highlight border
        g.setColour (isHighlighted ? juce::Colour (0xffffe0b2) : juce::Colour (0xffffcc80));
        g.drawRoundedRectangle (bounds, cr, 1.2f);

        // Subtle top glass highlight
        g.setColour (juce::Colour (0x60ffffff));
        g.drawLine (bounds.getX() + 2.0f, bounds.getY() + 1.5f,
                    bounds.getRight() - 2.0f, bounds.getY() + 1.5f, 1.0f);
    }
    else
    {
        // OFF state: dark slate with beat grouping
        juce::Colour baseCol = isDownbeat ? juce::Colour (0xff2f3440) : juce::Colour (0xff242730);
        if (isHighlighted)
            baseCol = baseCol.brighter (0.18f);
        else if (isDown)
            baseCol = baseCol.darker (0.25f);

        g.setColour (baseCol);
        g.fillRoundedRectangle (bounds, cr);

        // Border
        juce::Colour borderCol = isHighlighted ? juce::Colour (0xff60a5fa)
                               : isDownbeat   ? juce::Colour (0xff4b5568)
                                              : juce::Colour (0xff373d49);
        g.setColour (borderCol);
        g.drawRoundedRectangle (bounds, cr, 1.0f);
    }
}

void MetuliferLookAndFeel::drawStepButtonText (juce::Graphics& g, juce::TextButton& button)
{
    const int stepNum = button.getButtonText().substring (5).getIntValue();
    const bool isOn = button.getToggleState();
    const bool isDownbeat = ((stepNum - 1) % 4 == 0);

    const float fontH = button.getWidth() > 22 ? 10.5f : 9.5f;
    g.setFont (juce::FontOptions (fontH, juce::Font::bold));

    if (isOn)
        g.setColour (juce::Colours::white);
    else if (isDownbeat)
        g.setColour (juce::Colour (0xffcbd5e1));
    else
        g.setColour (juce::Colour (0xff828c9b));

    g.drawText (juce::String (stepNum), button.getLocalBounds(),
                juce::Justification::centred, false);
}

void MetuliferLookAndFeel::drawStandardButtonBackground (juce::Graphics& g, juce::Button& button,
                                                         const juce::Colour& /*backgroundColour*/,
                                                         bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const auto text = button.getButtonText();
    float cr = 5.0f;

    juce::Colour topCol, botCol, borderCol;

    if (text == "Play" || text == "Stop")
    {
        cr = 6.0f;
        const bool isPlaying = (text == "Stop") || button.getToggleState();

        if (isPlaying) // Stop (Red)
        {
            topCol = isHighlighted ? juce::Colour (0xffef4444) : isDown ? juce::Colour (0xffb91c1c) : juce::Colour (0xffdc2626);
            botCol = isHighlighted ? juce::Colour (0xffdc2626) : isDown ? juce::Colour (0xff7f1d1d) : juce::Colour (0xff991b1b);
            borderCol = isHighlighted ? juce::Colour (0xfffca5a5) : juce::Colour (0xffef4444);
        }
        else // Play (Green)
        {
            topCol = isHighlighted ? juce::Colour (0xff22c55e) : isDown ? juce::Colour (0xff15803d) : juce::Colour (0xff16a34a);
            botCol = isHighlighted ? juce::Colour (0xff16a34a) : isDown ? juce::Colour (0xff14532d) : juce::Colour (0xff15803d);
            borderCol = isHighlighted ? juce::Colour (0xff86efac) : juce::Colour (0xff22c55e);
        }
    }
    else if (text == "On" || text == "Off")
    {
        const bool isOn = (text == "On") || button.getToggleState();

        if (isOn)
        {
            topCol = isHighlighted ? juce::Colour (0xff22c55e) : isDown ? juce::Colour (0xff15803d) : juce::Colour (0xff16a34a);
            botCol = isHighlighted ? juce::Colour (0xff16a34a) : isDown ? juce::Colour (0xff14532d) : juce::Colour (0xff15803d);
            borderCol = isHighlighted ? juce::Colour (0xff86efac) : juce::Colour (0xff22c55e);
        }
        else
        {
            topCol = isHighlighted ? juce::Colour (0xff373c47) : isDown ? juce::Colour (0xff1c1e24) : juce::Colour (0xff2d313a);
            botCol = isHighlighted ? juce::Colour (0xff2d313a) : isDown ? juce::Colour (0xff16171d) : juce::Colour (0xff22252c);
            borderCol = isHighlighted ? juce::Colour (0xff64748b) : juce::Colour (0xff475569);
        }
    }
    else if (text == "LPF" || text == "HPF")
    {
        const bool isActive = button.getToggleState();

        if (isActive)
        {
            topCol = isHighlighted ? juce::Colour (0xff3b82f6) : isDown ? juce::Colour (0xff1e40af) : juce::Colour (0xff2563eb);
            botCol = isHighlighted ? juce::Colour (0xff2563eb) : isDown ? juce::Colour (0xff172554) : juce::Colour (0xff1d4ed8);
            borderCol = isHighlighted ? juce::Colour (0xffbfdbfe) : juce::Colour (0xff60a5fa);
        }
        else
        {
            topCol = isHighlighted ? juce::Colour (0xff373c47) : isDown ? juce::Colour (0xff1c1e24) : juce::Colour (0xff2d313a);
            botCol = isHighlighted ? juce::Colour (0xff2d313a) : isDown ? juce::Colour (0xff16171d) : juce::Colour (0xff22252c);
            borderCol = isHighlighted ? juce::Colour (0xff64748b) : juce::Colour (0xff475569);
        }
    }
    else if (text == "C" || text == "P" || text == "Copy" || text == "Paste")
    {
        cr = 4.0f;
        topCol = isHighlighted ? juce::Colour (0xff3e4553) : isDown ? juce::Colour (0xff1a1d24) : juce::Colour (0xff2e333d);
        botCol = isHighlighted ? juce::Colour (0xff2e333d) : isDown ? juce::Colour (0xff14161c) : juce::Colour (0xff22262e);
        borderCol = isHighlighted ? juce::Colour (0xff60a5fa) : juce::Colour (0xff475569);
    }
    else if (text == "ARP")
    {
        cr = 4.0f;
        const bool isActive = button.getToggleState();
        if (isActive)
        {
            topCol = isHighlighted ? juce::Colour (0xff3b82f6) : isDown ? juce::Colour (0xff1e40af) : juce::Colour (0xff2563eb);
            botCol = isHighlighted ? juce::Colour (0xff2563eb) : isDown ? juce::Colour (0xff172554) : juce::Colour (0xff1d4ed8);
            borderCol = isHighlighted ? juce::Colour (0xffbfdbfe) : juce::Colour (0xff60a5fa);
        }
        else
        {
            topCol = isHighlighted ? juce::Colour (0xff3e4553) : isDown ? juce::Colour (0xff1a1d24) : juce::Colour (0xff2e333d);
            botCol = isHighlighted ? juce::Colour (0xff2e333d) : isDown ? juce::Colour (0xff14161c) : juce::Colour (0xff22262e);
            borderCol = isHighlighted ? juce::Colour (0xff60a5fa) : juce::Colour (0xff475569);
        }
    }
    else if (text == "REV" || text == "DLY" || text == "RDX")
    {
        cr = 3.0f;
        topCol = isHighlighted ? juce::Colour (0xff383e4d) : isDown ? juce::Colour (0xff181a20) : juce::Colour (0xff262a34);
        botCol = isHighlighted ? juce::Colour (0xff262a34) : isDown ? juce::Colour (0xff121418) : juce::Colour (0xff1d2028);
        borderCol = isHighlighted ? juce::Colour (0xff60a5fa) : juce::Colour (0xff475569);
    }
    else
    {
        // Mode ("Room"/"Hall", "PingPong"/"Stereo"), Wave ("Sin"/"Saw"...), Arp modes/chords, Inspect ("Inspect")
        topCol = isHighlighted ? juce::Colour (0xff3d4452) : isDown ? juce::Colour (0xff1a1d24) : juce::Colour (0xff2d323c);
        botCol = isHighlighted ? juce::Colour (0xff2d323c) : isDown ? juce::Colour (0xff14161b) : juce::Colour (0xff21252d);
        borderCol = isHighlighted ? juce::Colour (0xff38bdf8) : juce::Colour (0xff475569);
    }

    // Shadow
    g.setColour (juce::Colour (0x18000000));
    g.fillRoundedRectangle (bounds.translated (0.0f, 1.0f), cr);

    // Gradient Fill
    juce::ColourGradient grad (topCol, bounds.getCentreX(), bounds.getY(),
                               botCol, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, cr);

    // Border
    g.setColour (borderCol);
    g.drawRoundedRectangle (bounds, cr, 1.2f);

    // Subtle top highlight
    g.setColour (juce::Colour (0x35ffffff));
    g.drawLine (bounds.getX() + cr, bounds.getY() + 1.0f,
                bounds.getRight() - cr, bounds.getY() + 1.0f, 1.0f);
}

void MetuliferLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool /*shouldDrawButtonAsDown*/)
{
    if (findButtonImage (button).isValid())
        return;

    const auto text = button.getButtonText();

    if (text.startsWith ("step "))
    {
        drawStepButtonText (g, button);
        return;
    }

    const bool isOn = button.getToggleState();
    float fontH = 12.0f;
    auto fontStyle = juce::Font::bold;
    juce::Colour textCol (0xffffffff);

    if (text == "Play" || text == "Stop")
    {
        fontH = 15.0f;
        textCol = juce::Colours::white;
    }
    else if (text == "On" || text == "Off")
    {
        textCol = (text == "On" || isOn) ? juce::Colours::white : juce::Colour (0xff94a3b8);
    }
    else if (text == "LPF" || text == "HPF")
    {
        textCol = isOn ? juce::Colours::white : juce::Colour (0xff94a3b8);
    }
    else if (text == "C" || text == "P" || text == "Copy" || text == "Paste")
    {
        fontH = 12.0f;
        textCol = shouldDrawButtonAsHighlighted ? juce::Colours::white : juce::Colour (0xffe2e8f0);
    }
    else if (text == "ARP")
    {
        fontH = 10.5f;
        textCol = isOn ? juce::Colours::white : (shouldDrawButtonAsHighlighted ? juce::Colour (0xffcbd5e1) : juce::Colour (0xff94a3b8));
    }
    else if (text == "REV" || text == "DLY" || text == "RDX")
    {
        fontH = 10.0f;
        textCol = shouldDrawButtonAsHighlighted ? juce::Colours::white : juce::Colour (0xffe2e8f0);
    }
    else if (text == "Inspect")
    {
        fontH = 11.0f;
        fontStyle = juce::Font::plain;
        textCol = shouldDrawButtonAsHighlighted ? juce::Colours::white : juce::Colour (0xffcbd5e1);
    }
    else
    {
        // Mode ("Room"/"Hall"), Wave ("Sin"/"Saw"...)
        fontH = 12.0f;
        textCol = shouldDrawButtonAsHighlighted ? juce::Colour (0xffe0f2fe) : juce::Colour (0xfff1f5f9);
    }

    g.setFont (juce::FontOptions (fontH, fontStyle));
    g.setColour (textCol);
    g.drawText (text, button.getLocalBounds(), juce::Justification::centred, false);
}
