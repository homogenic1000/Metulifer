#include "ReverbPanel.h"
#include "Skin.h"

ReverbPanel::ReverbPanel()
{
    titleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (titleLabel);

    onOffButton.setClickingTogglesState (true);
    onOffButton.setToggleState (false, juce::dontSendNotification);
    onOffButton.setButtonText ("Off");
    onOffButton.onClick = [this]
    {
        isEnabled = onOffButton.getToggleState();
        onOffButton.setButtonText (isEnabled ? "On" : "Off");
        if (onPowerToggled)
            onPowerToggled (isEnabled);
    };
    addAndMakeVisible (onOffButton);

    modeButton.setButtonText ("Room");
    modeButton.onClick = [this]
    {
        modeIndex = (modeIndex + 1) % 2;
        modeButton.setButtonText (modeIndex == 0 ? "Room" : "Hall");
        if (onModeToggled)
            onModeToggled (modeIndex);
    };
    addAndMakeVisible (modeButton);

    mixLabel.setFont (juce::FontOptions (11.0f));
    mixLabel.setJustificationType (juce::Justification::centred);
    mixLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (mixLabel);

    mixKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    mixKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    mixKnob.setName ("Reverb_Mix");
    mixKnob.setRange (0.0, 100.0, 1.0);
    mixKnob.setValue (30.0, juce::dontSendNotification);
    addAndMakeVisible (mixKnob);
}

void ReverbPanel::paint (juce::Graphics& g)
{
    Skin::drawPanel (g, "panel_reverb", getLocalBounds().toFloat().reduced (0.5f),
                     juce::Colour (0xffdedede), 6.0f);
}

void ReverbPanel::resized()
{
    auto area = getLocalBounds().reduced (4);

    titleLabel.setBounds (area.removeFromTop (22));

    auto buttons = area.removeFromTop (49);
    onOffButton.setBounds (buttons.getX(), buttons.getY(), 50, 49);
    modeButton.setBounds (buttons.getX() + 55, buttons.getY(), 50, 49);

    area.removeFromTop (16);
    auto knobArea = area.removeFromTop (100);
    mixKnob.setBounds (knobArea.getX() + 10, knobArea.getY(), 86, 86);
    mixLabel.setBounds (knobArea.getX(), knobArea.getBottom() - 4, 105, 14);
}

void ReverbPanel::setEnabledState (bool on)
{
    isEnabled = on;
    onOffButton.setToggleState (on, juce::dontSendNotification);
    onOffButton.setButtonText (on ? "On" : "Off");
}

void ReverbPanel::setModeIndex (int mode)
{
    modeIndex = juce::jlimit (0, 1, mode);
    modeButton.setButtonText (modeIndex == 0 ? "Room" : "Hall");
}
