#include "LFOPanel.h"
#include "Skin.h"

static const char* waveName (int index)
{
    switch (index)
    {
        case 1:  return "Saw";
        case 2:  return "Sqr";
        case 3:  return "Tri";
        default: return "Sin";
    }
}

LFOPanel::LFOPanel (const juce::String& name)
    : panelName (name)
{
    titleLabel.setText (name, juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (titleLabel);

    waveButton.setButtonText (waveName (0));
    waveNameText = waveName (0);
    waveButton.onClick = [this]
    {
        waveIndex = (waveIndex + 1) % 4;
        waveButton.setButtonText (waveName (waveIndex));
        waveNameText = waveName (waveIndex);
        repaint();
        if (onWaveClicked)
            onWaveClicked (waveIndex);
    };
    addAndMakeVisible (waveButton);

    onOffButton.setClickingTogglesState (true);
    onOffButton.setButtonText ("Off");
    onOffButton.onClick = [this]
    {
        on = onOffButton.getToggleState();
        onOffButton.setButtonText (on ? "On" : "Off");
        repaint();
        if (onPowerClicked)
            onPowerClicked (on);
    };
    addAndMakeVisible (onOffButton);

    auto setup = [] (juce::Slider& s, const juce::String& n)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        s.setName (n);
    };

    setup (rateKnob, name + "_Rate");
    setup (offsetKnob, name + "_Offset");
    setup (depthKnob, name + "_Depth");
    addAndMakeVisible (rateKnob);
    addAndMakeVisible (offsetKnob);
    addAndMakeVisible (depthKnob);

    for (auto* l : { &rateLabel, &offsetLabel, &depthLabel })
    {
        l->setFont (juce::FontOptions (11.0f));
        l->setJustificationType (juce::Justification::centredLeft);
        l->setColour (juce::Label::textColourId, juce::Colours::black);
        addAndMakeVisible (l);
    }
}

void LFOPanel::paint (juce::Graphics& g)
{
    Skin::drawPanel (g, "panel_lfo", getLocalBounds().toFloat().reduced (0.5f),
                     juce::Colour (0xffdedede), 6.0f);

    const bool isLfo1 = panelName.contains ("1");
    const juce::Colour themeColor = isLfo1 ? juce::Colour (0xffff8c24) : juce::Colour (0xff00d4ff);

    // Mini OLED Screen
    const auto oled = getOledBounds();
    g.setColour (juce::Colour (0xff0a0d12));
    g.fillRoundedRectangle (oled, 4.0f);
    g.setColour (juce::Colour (0xff1e2638));
    g.drawRoundedRectangle (oled, 4.0f, 1.0f);

    // Subtle OLED scanline texture
    g.setColour (juce::Colour (0x0affffff));
    for (float y = oled.getY() + 3.0f; y < oled.getBottom() - 2.0f; y += 3.0f)
        g.drawHorizontalLine ((int) y, oled.getX() + 2.0f, oled.getRight() - 2.0f);

    const bool hasTarget = (destinationText.isNotEmpty() && destinationText != "NO TARGET");

    // Line 1: Header / Rate
    g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xff64748b));
    g.drawText ("TARGET", oled.getX() + 5.0f, oled.getY() + 3.0f, 40.0f, 11.0f,
                juce::Justification::centredLeft, false);

    // Wave status text on right
    g.drawText (waveNameText + " " + juce::String (currentRateHz, 1) + "Hz",
                oled.getX() + 42.0f, oled.getY() + 3.0f, oled.getWidth() - 47.0f, 11.0f,
                juce::Justification::centredRight, false);

    // Line 2: Target Value (Backlit OLED style)
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    if (hasTarget)
    {
        g.setColour (themeColor.withAlpha (0.35f));
        g.drawText (destinationText.toUpperCase(),
                    oled.getX() + 5.0f, oled.getY() + 15.0f, oled.getWidth() - 10.0f, 13.0f,
                    juce::Justification::centredLeft, true);
        g.setColour (themeColor);
        g.drawText (destinationText.toUpperCase(),
                    oled.getX() + 5.0f, oled.getY() + 15.0f, oled.getWidth() - 10.0f, 13.0f,
                    juce::Justification::centredLeft, true);
    }
    else
    {
        g.setColour (juce::Colour (0xff475569));
        g.drawText ("[ UNLINKED ]",
                    oled.getX() + 5.0f, oled.getY() + 15.0f, oled.getWidth() - 10.0f, 13.0f,
                    juce::Justification::centredLeft, false);
    }

    // Mini activity dot inside OLED
    if (on)
    {
        const float dotX = oled.getRight() - 10.0f;
        const float dotY = oled.getY() + 21.0f - (currentLfoValue * 5.0f);
        g.setColour (themeColor);
        g.fillEllipse (dotX - 2.5f, dotY - 2.5f, 5.0f, 5.0f);
    }

    // Bottom Output Jack
    const auto jack = getJackBounds();
    g.setColour (juce::Colour (0xff2d3139));
    g.fillEllipse (jack.expanded (1.5f));
    g.setColour (juce::Colour (0xff4b5563));
    g.drawEllipse (jack.expanded (1.5f), 1.0f);

    g.setColour (juce::Colour (0xff0f1115));
    g.fillEllipse (jack);
    g.setColour (themeColor);
    g.drawEllipse (jack.reduced (1.0f), 1.2f);

    // Activity LED Indicator
    const float ledX = jack.getRight() + 6.0f;
    const float ledY = jack.getCentreY() - 3.5f;
    const juce::Rectangle<float> ledRect (ledX, ledY, 7.0f, 7.0f);

    if (on)
    {
        const float brightness = juce::jlimit (0.25f, 1.0f, (currentLfoValue + 1.0f) * 0.5f);
        g.setColour (themeColor.withAlpha (brightness * 0.45f));
        g.fillEllipse (ledRect.expanded (2.5f));
        g.setColour (themeColor.withAlpha (brightness));
        g.fillEllipse (ledRect);
        g.setColour (juce::Colours::white.withAlpha (brightness * 0.8f));
        g.fillEllipse (ledRect.reduced (1.5f));
    }
    else
    {
        g.setColour (juce::Colour (0xff1f242d));
        g.fillEllipse (ledRect);
        g.setColour (juce::Colour (0xff374151));
        g.drawEllipse (ledRect, 0.8f);
    }

    // OUT label
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xff1e293b));
    g.drawText ("OUT", ledX + 11.0f, jack.getY(), 26.0f, jack.getHeight(),
                juce::Justification::centredLeft, false);
}

juce::Rectangle<float> LFOPanel::getJackBounds() const
{
    return { 7.0f, (float) getHeight() - 20.0f, 13.0f, 13.0f };
}

juce::Rectangle<float> LFOPanel::getOledBounds() const
{
    return { 6.0f, 47.0f, (float) getWidth() - 12.0f, 32.0f };
}

void LFOPanel::resized()
{
    titleLabel.setBounds (4, 4, getWidth() - 8, 16);

    const int btnW = (getWidth() - 13) / 2;
    waveButton.setBounds (5, 21, btnW, 23);
    onOffButton.setBounds (8 + btnW, 21, btnW, 23);

    auto row = [this] (int y, juce::Slider& knob, juce::Label& label)
    {
        label.setBounds (6, y, 38, 36);
        knob.setBounds (46, y, 38, 38);
    };

    row (82,  rateKnob,   rateLabel);
    row (120, offsetKnob, offsetLabel);
    row (158, depthKnob,  depthLabel);
}

void LFOPanel::setWaveIndex (int index)
{
    waveIndex = juce::jlimit (0, 3, index);
    waveButton.setButtonText (waveName (waveIndex));
    waveNameText = waveName (waveIndex);
    repaint();
}

void LFOPanel::setEnabled (bool shouldBeOn)
{
    on = shouldBeOn;
    onOffButton.setToggleState (on, juce::dontSendNotification);
    onOffButton.setButtonText (on ? "On" : "Off");
    repaint();
}

void LFOPanel::setDestinationSummary (const juce::String& summary)
{
    if (destinationText != summary)
    {
        destinationText = summary;
        repaint();
    }
}

void LFOPanel::setRateHz (float hz)
{
    if (std::abs (currentRateHz - hz) > 0.05f)
    {
        currentRateHz = hz;
        repaint();
    }
}

void LFOPanel::setLfoActivity (float lfoVal)
{
    currentLfoValue = lfoVal;
    repaint();
}
