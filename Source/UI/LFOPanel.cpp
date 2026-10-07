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
    waveButton.onClick = [this]
    {
        waveIndex = (waveIndex + 1) % 4;
        waveButton.setButtonText (waveName (waveIndex));
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

    const auto jack = getJackBounds();
    g.setColour (juce::Colour (0xff4a4a4a));
    g.fillEllipse (jack);
    g.setColour (juce::Colour (0xff9a9a9a));
    g.drawEllipse (jack.reduced (0.5f), 1.0f);
}

juce::Rectangle<float> LFOPanel::getJackBounds() const
{
    return { 7.0f, (float) getHeight() - 19.0f, 12.0f, 12.0f };
}

void LFOPanel::resized()
{
    auto area = getLocalBounds().reduced (4);

    titleLabel.setBounds (area.removeFromTop (20));

    auto buttons = area.removeFromTop (49);
    const int btnW = (buttons.getWidth() - 5) / 2;
    waveButton.setBounds (buttons.getX(), buttons.getY(), btnW, 49);
    onOffButton.setBounds (buttons.getX() + btnW + 5, buttons.getY(), btnW, 49);

    area.removeFromTop (4);
    area.removeFromBottom (16); // place pour le jack

    auto row = [this, &area] (juce::Slider& knob, juce::Label& label)
    {
        auto r = area.removeFromTop (40);
        label.setBounds (r.getX(), r.getY(), 34, r.getHeight());
        knob.setBounds (r.getX() + 36, r.getY(), 40, 40);
        area.removeFromTop (2);
    };

    row (rateKnob, rateLabel);
    row (offsetKnob, offsetLabel);
    row (depthKnob, depthLabel);
}

void LFOPanel::setWaveIndex (int index)
{
    waveIndex = juce::jlimit (0, 3, index);
    waveButton.setButtonText (waveName (waveIndex));
}

void LFOPanel::setEnabled (bool shouldBeOn)
{
    on = shouldBeOn;
    onOffButton.setToggleState (on, juce::dontSendNotification);
    onOffButton.setButtonText (on ? "On" : "Off");
}
